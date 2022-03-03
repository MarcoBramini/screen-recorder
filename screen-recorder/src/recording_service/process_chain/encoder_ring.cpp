#include <fmt/core.h>
#include "encoder_ring.h"
#include "../error.h"
#include "process_context.h"

extern "C" {
#include <libavdevice/avdevice.h>
}

/// Initializes the encoder
EncoderChainRing::EncoderChainRing(AVStream *inputStream, AVStream *outputStream, const EncoderConfig &config)
        : inputStream(inputStream), outputStream(outputStream), lastEncodedDTS(-1) {
    // Find encoder for output stream
    auto outputStreamCodec = avcodec_find_encoder(config.codecID);
    if (!outputStreamCodec) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {},
                                           fmt::format("error finding encoder '{}'", config.codecID)));
    }

    // Allocate context for the encoder
    encoderContext = std::unique_ptr<AVCodecContext, FFMpegObjectsDeleter>(avcodec_alloc_context3(outputStreamCodec));
    if (!encoderContext) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, "error allocating context for the encoder"));
    }

    // Set encoder options
    int ret;
    for (const auto &option: config.encoderOptions) {
        ret = av_opt_set(encoderContext->priv_data, option.first.c_str(), option.second.c_str(), 0);
        if (ret < 0) {
            throw std::runtime_error(Error::build_error_message(__FUNCTION__, {}, fmt::format(
                    "error setting '{}' encoder option to value '{}' ({})",
                    option.first, option.second, Error::unpackAVError(ret))));
        }
    }

    switch (config.codecType) {
        case AVMEDIA_TYPE_VIDEO:
            encoderContext->height = config.height;
            encoderContext->width = config.width;
            encoderContext->pix_fmt = config.pixelFormat;
            encoderContext->bit_rate = config.bitRate;
            encoderContext->time_base = {1, config.frameRate};
            encoderContext->sample_aspect_ratio = inputStream->codecpar->sample_aspect_ratio;
            encoderContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            break;
        case AVMEDIA_TYPE_AUDIO:
        default:
            encoderContext->channels = config.channels;
            encoderContext->channel_layout = config.channelLayout;
            encoderContext->sample_rate = config.sampleRate;
            encoderContext->sample_fmt = config.sampleFormat;
            encoderContext->bit_rate = config.bitRate;
            encoderContext->time_base = {1, config.sampleRate};
            encoderContext->strict_std_compliance = config.strictStdCompliance;
            break;
    }

    // Set codec to automatically determine how many threads suits best for the decoding job
    encoderContext->thread_count = 0;
    if (outputStreamCodec->capabilities | AV_CODEC_CAP_FRAME_THREADS)
        encoderContext->thread_type = FF_THREAD_FRAME;
    else if (outputStreamCodec->capabilities | AV_CODEC_CAP_SLICE_THREADS)
        encoderContext->thread_type = FF_THREAD_SLICE;
    else
        encoderContext->thread_count = 1; //don't use multithreading

    // Open encoder
    ret = avcodec_open2(encoderContext.get(), outputStreamCodec, nullptr);
    if (ret < 0) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {},
                                           fmt::format("error opening encoder ({})", Error::unpackAVError(ret))));
    }

    // Copy encoder parameter to stream
    ret = avcodec_parameters_from_context(outputStream->codecpar, encoderContext.get());
    if (ret < 0) {
        throw std::runtime_error(Error::build_error_message(__FUNCTION__, {},
                                                            fmt::format(
                                                                    "error copying encoder parameters to the stream ({})",
                                                                    Error::unpackAVError(ret))));
    }
}

/// Flushes the remaining encoder frames
void EncoderChainRing::flush() {
    execute(nullptr, nullptr);
}

/// Processes an input frame and passes the encoded packet to the next ring.
/// Flushing is done by setting null parameters.
void EncoderChainRing::execute(ProcessContext *processContext, AVFrame *inputFrame) {
    // Null processContext and inputFrame means a flush has been requested. No need to calculate frame stuff.
    if (processContext && inputFrame) {
        // Calculate the encoder frame PTS
        inputFrame->pict_type = AV_PICTURE_TYPE_NONE;
        inputFrame->pts = av_rescale_q(processContext->sourcePacketPts,
                                       inputStream->time_base,
                                       encoderContext->time_base);
    }

    auto encodedPacket = std::unique_ptr<AVPacket, FFMpegObjectsDeleter>(av_packet_alloc());
    if (!encodedPacket) {
        throw std::runtime_error(Error::build_error_message(__FUNCTION__, {}, "error allocating packet"));
    }

    int response = avcodec_send_frame(encoderContext.get(), inputFrame);
    if (response < 0) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {},
                                           fmt::format("error sending packet to the encoder ({})",
                                                       Error::unpackAVError(response))));
    }

    while (response >= 0) {
        response = avcodec_receive_packet(encoderContext.get(), encodedPacket.get());
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            break;
        } else if (response < 0) {
            throw std::runtime_error(
                    Error::build_error_message(__FUNCTION__, {},
                                               fmt::format("error receiving packet from the encoder ({})",
                                                           Error::unpackAVError(response))));
        }

        encodedPacket->stream_index = outputStream->index;

        av_packet_rescale_ts(encodedPacket.get(), encoderContext->time_base, outputStream->time_base);

        if (encodedPacket->dts <= lastEncodedDTS) {
            continue;
        }
        lastEncodedDTS = encodedPacket->dts;

        // Pass the encoded packet to the next ring
        next->execute(processContext, encodedPacket.get());
    }
}
