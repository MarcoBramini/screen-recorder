#include <fmt/core.h>
#include "swresample_filter_ring.h"
#include "../error.h"
#include "../recording_service.h"

SWResampleFilterRing::SWResampleFilterRing(SWResampleConfig swResampleConfig) : config(swResampleConfig) {
    // Allocate audio converter context
    swrContext = swr_alloc_set_opts(nullptr,
                                    config.outputChannelLayout,
                                    config.outputSampleFormat,
                                    config.outputSampleRate,
                                    config.inputChannelLayout,
                                    config.inputSampleFormat,
                                    config.inputSampleRate,
                                    0, nullptr);
    if (!swrContext) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, "error allocating audio converter context"));
    }

    // Initialize the audio converter
    int ret = swr_init(swrContext);
    if (ret < 0) {
        throw std::runtime_error(Error::build_error_message(__FUNCTION__, {},
                                                            fmt::format("error initializing audio converter ({})",
                                                                        Error::unpackAVError(ret))));
    }

    // Allocate a 2 seconds FIFO buffer for converting
    outputBuffer = av_audio_fifo_alloc(OUTPUT_AUDIO_SAMPLE_FMT,
                                       config.inputChannels,
                                       config.outputSampleRate * 2);
    if (!outputBuffer) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, "error allocating audio converter buffer"));
    }
}

void SWResampleFilterRing::execute(ProcessContext *processContext, AVFrame *inputFrame) {
    uint8_t **audioData;
    int ret = av_samples_alloc_array_and_samples(&audioData, nullptr, config.outputChannels,
                                                 inputFrame->nb_samples, OUTPUT_AUDIO_SAMPLE_FMT, 0);
    if (ret < 0) {
        throw std::runtime_error("Fail to alloc samples by av_samples_alloc_array_and_samples.");
    }

    ret = swr_convert(swrContext, audioData, inputFrame->nb_samples,
                      (const uint8_t **) (inputFrame->extended_data),
                      inputFrame->nb_samples);
    if (ret < 0) {
        throw std::runtime_error("Fail to swr_convert.");
    }

    if (av_audio_fifo_space(outputBuffer) < inputFrame->nb_samples)
        throw std::runtime_error("audio buffer is too small.");

    ret = av_audio_fifo_write(outputBuffer, (void **) audioData, inputFrame->nb_samples);
    if (ret < 0) {
        throw std::runtime_error("Fail to write fifo");
    }

    av_freep(&audioData[0]);

    AVFrame *convertedFrame = av_frame_alloc();
    if (!convertedFrame) {
        throw std::runtime_error("Error allocating new frame");
    }

    int64_t framePts = processContext->sourcePacketPts;
    int64_t frameOffset = 0;
    while (av_audio_fifo_size(outputBuffer) >= config.outputFrameSize) {
        // Build frame from buffer
        convertedFrame->nb_samples = config.outputFrameSize;
        convertedFrame->channels = config.outputChannels;
        convertedFrame->channel_layout = config.outputChannelLayout;
        convertedFrame->format = config.outputSampleFormat;
        convertedFrame->sample_rate = config.outputSampleRate;

        // Calculate converted frame pts
        processContext->sourcePacketPts = framePts + frameOffset;
        frameOffset += av_rescale_q(config.outputFrameSize, config.outputTimeBase, config.inputTimeBase);

        if (av_frame_get_buffer(convertedFrame, 0) < 0) {
            throw std::runtime_error("Error reading from audio buffer");
        }

        if (av_audio_fifo_read(outputBuffer, (void **) convertedFrame->data, config.outputFrameSize) < 0) {
            throw std::runtime_error("Error reading from audio buffer");
        }

        if (std::holds_alternative<FilterChainRing *>(getNext())) {
            std::get<FilterChainRing *>(getNext())->execute(processContext, convertedFrame);
        } else {
            std::get<EncoderChainRing *>(getNext())->execute(processContext, convertedFrame);
        }

        av_frame_unref(convertedFrame);
    }
    av_frame_free(&convertedFrame);

}
