#include "swresample_filter_ring.h"
#include <fmt/core.h>
#include "../error.h"

/// Initializes a resample filter, used to convert an input audio decoded frame to the output format
SWResampleFilterRing::SWResampleFilterRing(SWResampleConfig swResampleConfig) : config(swResampleConfig) {
    // Allocate audio converter context
    swrContext = std::unique_ptr<SwrContext, FFMpegObjectsDeleter>(swr_alloc_set_opts(nullptr,
                                                                                      config.outputChannelLayout,
                                                                                      config.outputSampleFormat,
                                                                                      config.outputSampleRate,
                                                                                      config.inputChannelLayout,
                                                                                      config.inputSampleFormat,
                                                                                      config.inputSampleRate,
                                                                                      0, nullptr));
    if (!swrContext) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, "error allocating audio converter context"));
    }

    // Initialize the audio converter
    int ret = swr_init(swrContext.get());
    if (ret < 0) {
        throw std::runtime_error(Error::build_error_message(__FUNCTION__, {},
                                                            fmt::format("error initializing audio converter ({})",
                                                                        Error::unpackAVError(ret))));
    }

    // Allocate a 2 seconds FIFO buffer for converting
    outputBuffer = std::unique_ptr<AVAudioFifo, FFMpegObjectsDeleter>(av_audio_fifo_alloc(config.outputSampleFormat,
                                                                                          config.inputChannels,
                                                                                          config.outputSampleRate * 2));
    if (!outputBuffer) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, "error allocating audio converter buffer"));
    }
}

/// Processes an input frame and passes it to the next ring
void SWResampleFilterRing::execute(ProcessContext *processContext, AVFrame *inputFrame) {

    uint8_t **tempBuffer;
    int ret = av_samples_alloc_array_and_samples(&tempBuffer, nullptr, config.outputChannels,
                                                 inputFrame->nb_samples, config.outputSampleFormat, 0);
    if (ret < 0) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {},
                                           fmt::format("error allocating input audio buffers ({})",
                                                       Error::unpackAVError(ret))));
    }

    auto audioData = std::unique_ptr<uint8_t **, FFMpegObjectsDeleter>(&tempBuffer);

    ret = swr_convert(swrContext.get(), *audioData, inputFrame->nb_samples,
                      (const uint8_t **) (inputFrame->extended_data),
                      inputFrame->nb_samples);
    if (ret < 0) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {},
                                           fmt::format("error converting the input frame ({})",
                                                       Error::unpackAVError(ret))));
    }

    if (av_audio_fifo_space(outputBuffer.get()) < inputFrame->nb_samples)
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, "the provided input audio buffer is too small"));

    ret = av_audio_fifo_write(outputBuffer.get(), (void **) *audioData, inputFrame->nb_samples);
    if (ret < 0) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {},
                                           fmt::format("error writing to the output audio buffer ({})",
                                                       Error::unpackAVError(ret))));
    }

    int64_t framePts = processContext->sourcePacketPts;
    int64_t frameOffset = 0;
    while (av_audio_fifo_size(outputBuffer.get()) >= config.outputFrameSize) {
        auto convertedFrame = std::unique_ptr<AVFrame, FFMpegObjectsDeleter>(av_frame_alloc());
        if (!convertedFrame) {
            throw std::runtime_error(
                    Error::build_error_message(__FUNCTION__, {}, "error allocating a new frame"));
        }

        // Build frame from buffer
        convertedFrame->nb_samples = config.outputFrameSize;
        convertedFrame->channels = config.outputChannels;
        convertedFrame->channel_layout = config.outputChannelLayout;
        convertedFrame->format = config.outputSampleFormat;
        convertedFrame->sample_rate = config.outputSampleRate;

        // Calculate converted frame pts
        processContext->sourcePacketPts = framePts + frameOffset;
        frameOffset += av_rescale_q(config.outputFrameSize, config.outputTimeBase, config.inputTimeBase);

        ret = av_frame_get_buffer(convertedFrame.get(), 0);
        if (ret < 0) {
            throw std::runtime_error(
                    Error::build_error_message(__FUNCTION__, {},
                                               fmt::format("error allocating the converted frame audio buffer ({})",
                                                           Error::unpackAVError(ret))));
        }

        ret = av_audio_fifo_read(outputBuffer.get(), (void **) convertedFrame->data, config.outputFrameSize);
        if (ret < 0) {
            throw std::runtime_error(
                    Error::build_error_message(__FUNCTION__, {},
                                               fmt::format("error reading from the converted frame audio buffer ({})",
                                                           Error::unpackAVError(ret))));
        }

        // Pass the converted frame to the next ring
        if (std::holds_alternative<std::shared_ptr<FilterChainRing>>(getNext())) {
            std::get<std::shared_ptr<FilterChainRing>>(getNext())->execute(processContext, convertedFrame.get());
        } else {
            std::get<std::shared_ptr<EncoderChainRing>>(getNext())->execute(processContext, convertedFrame.get());
        }
    }
}
