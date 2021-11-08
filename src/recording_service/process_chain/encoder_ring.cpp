#include <fmt/core.h>
#include "encoder_ring.h"
#include "../error.h"
#include "process_context.h"
#include <iostream>

extern "C" {
#include <libavdevice/avdevice.h>
}

EncoderChainRing::EncoderChainRing(AVStream * inputStream, AVStream* outputStream, const EncoderConfig &config)
        : inputStream(inputStream), outputStream(outputStream), outputStreamCodec(nullptr), encoderContext(nullptr), lastEncodedDTS(-1),
          next(nullptr) {

    // Initialize encoder
    init_encoder(config);
}

/// Initializes the encoder for the output stream.
void EncoderChainRing::init_encoder(const EncoderConfig &config) {
    // Find encoder for output stream
    outputStreamCodec = avcodec_find_encoder(config.codecID);
    if (!outputStreamCodec) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, fmt::format("error finding encoder '{}'", config.codecID)));
    }

    // Allocate context for the encoder
    encoderContext = avcodec_alloc_context3(outputStreamCodec);
    if (!encoderContext) {
        throw std::runtime_error(Error::build_error_message(__FUNCTION__, {}, "error allocating context for the encoder"));
    }

    int ret;
    for (const auto &option:config.encoderOptions) {
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

    // TODO: is this useful?
    // set codec to automatically determine how many threads suits best for the decoding job
    encoderContext->thread_count = 0;

    if (outputStreamCodec->capabilities | AV_CODEC_CAP_FRAME_THREADS)
        encoderContext->thread_type = FF_THREAD_FRAME;
    else if (outputStreamCodec->capabilities | AV_CODEC_CAP_SLICE_THREADS)
        encoderContext->thread_type = FF_THREAD_SLICE;
    else
        encoderContext->thread_count = 1; //don't use multithreading

    // Open encoder
    ret = avcodec_open2(encoderContext, outputStreamCodec, nullptr);
    if (ret < 0) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, fmt::format("error opening encoder ({})", Error::unpackAVError(ret))));
    }

    // Copy encoder parameter to stream
    ret = avcodec_parameters_from_context(outputStream->codecpar, encoderContext);
    if (ret < 0) {
        throw std::runtime_error(Error::build_error_message(__FUNCTION__, {},
                                                     fmt::format("error copying encoder parameters to the stream ({})",
                                                                 Error::unpackAVError(ret))));
    }
}

void EncoderChainRing::flush(){
    execute(nullptr, nullptr);
}

void EncoderChainRing::execute(ProcessContext* processContext, AVFrame *inputFrame) {
    // Calculate frame PTS
    if (processContext) { // Are we flushing?
        inputFrame->pict_type = AV_PICTURE_TYPE_NONE;
        inputFrame->pts = av_rescale_q(processContext->sourcePacketPts,
                                       inputStream->time_base,
                                       encoderContext->time_base);
    }

    AVPacket *encodedPacket = av_packet_alloc();
    if (!encodedPacket) {
        throw std::runtime_error(Error::build_error_message(__FUNCTION__, {}, "error allocating packet"));
    }

    int response = avcodec_send_frame(encoderContext, inputFrame);

    while (response >= 0) {
        response = avcodec_receive_packet(encoderContext, encodedPacket);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            break;
        } else if (response < 0) {
            throw std::runtime_error(Error::build_error_message(__FUNCTION__, {}, "error receiving packet from the encoder"));
        }

        encodedPacket->stream_index = outputStream->index;

        av_packet_rescale_ts(encodedPacket, encoderContext->time_base, outputStream->time_base);

        if (encodedPacket->dts <= lastEncodedDTS) {
            continue;
        }
        lastEncodedDTS = encodedPacket->dts;

        next->execute(processContext, encodedPacket);
    }
    av_packet_unref(encodedPacket);
    av_packet_free(&encodedPacket);
}
