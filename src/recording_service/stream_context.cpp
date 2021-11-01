//
// Created by Marco Bramini on 01/11/21.
//

#include "stream_context.h"
#include "error.h"
#include <fmt/core.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
}

/// Creates a StreamContext starting from an existing stream.
/// It also initializes the decoder for the stream.
StreamContext StreamContext::from_input_stream(AVStream *stream) {
    StreamContext streamContext{};
    streamContext.avs = stream;
    streamContext.type = stream->codecpar->codec_type;

    // Init decoder
    streamContext.init_decoder();

    return streamContext;
}

/// Allocates a new stream for the provided device and returns a newly created StreamContext.
StreamContext StreamContext::new_output_stream(AVFormatContext *avfc, const EncoderConfig &encoderConfig) {
    if (avfc == nullptr) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "provided context is null"));
    }

    StreamContext streamContext{};

    AVStream *avs = avformat_new_stream(avfc, nullptr);
    if (!avs) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error allocating output AVStream"));
    }

    streamContext.avs = avs;
    streamContext.type = avs->codecpar->codec_type;

    // Init encoder
    streamContext.init_encoder(encoderConfig);

    return streamContext;
}


/// Initializes the decoder for the current stream.
void StreamContext::init_decoder() {
    if (avs == nullptr) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "stream not initialized"));
    }

    avc = avcodec_find_decoder(avs->codecpar->codec_id);
    if (!avc) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "no decoder found for the input stream"));
    }

    avcc = avcodec_alloc_context3(avc);
    if (!avcc) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error allocating context for the decoder"));
    }

    int ret;
    ret = avcodec_parameters_to_context(avcc, avs->codecpar);
    if (ret < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {},
                                                     fmt::format("error copying decoder parameters to the context ({})",
                                                                 unpackAVError(ret))));
    }

    ret = avcodec_open2(avcc, avc, nullptr);
    if (ret < 0) {
        throw std::runtime_error(
                build_error_message(__FUNCTION__, {}, fmt::format("error opening decoder ({})", unpackAVError(ret))));
    }
}


/// Initializes the encoder for the output stream.
void StreamContext::init_encoder(const EncoderConfig &config) {
    // Find encoder for output stream
    avc = avcodec_find_encoder(config.codecID);
    if (!avc) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error finding encoder 'AV_CODEC_ID_H264'"));
    }

    // Allocate context for the encoder
    avcc = avcodec_alloc_context3(avc);
    if (!avcc) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error allocating context for the encoder"));
    }

    int ret;
    for (const auto &option:config.encoderOptions) {
        ret = av_opt_set(avcc->priv_data, option.first.c_str(), option.second.c_str(), 0);
        if (ret < 0) {
            throw std::runtime_error(build_error_message(__FUNCTION__, {}, fmt::format(
                    "error setting '{}' encoder option to value '{}' ({})",
                    option.first, option.second, unpackAVError(ret))));
        }
    }

    switch (config.codecType) {
        case AVMEDIA_TYPE_VIDEO:
            avcc->height = config.height;
            avcc->width = config.width;
            avcc->pix_fmt = config.pixelFormat;
            avcc->bit_rate = config.bitRate;
            avcc->framerate = {config.frameRate, 1};
            avcc->time_base = {1, config.frameRate};
            break;
        case AVMEDIA_TYPE_AUDIO:
        default:
            avcc->channels = config.channels;
            avcc->channel_layout = config.channels;
            avcc->sample_rate = config.sampleRate;
            avcc->sample_fmt = config.sampleFormat;
            avcc->bit_rate = config.bitRate;
            avcc->time_base = {1, config.sampleRate};
            avcc->strict_std_compliance = config.strictStdCompliance;
            break;
    }

    // TODO: is this useful?
    // set codec to automatically determine how many threads suits best for the decoding job
    avcc->thread_count = 0;

    if (avc->capabilities | AV_CODEC_CAP_FRAME_THREADS)
        avcc->thread_type = FF_THREAD_FRAME;
    else if (avc->capabilities | AV_CODEC_CAP_SLICE_THREADS)
        avcc->thread_type = FF_THREAD_SLICE;
    else
        avcc->thread_count = 1; //don't use multithreading

    // Open encoder
    ret = avcodec_open2(avcc, avc, nullptr);
    if (ret < 0) {
        throw std::runtime_error(
                build_error_message(__FUNCTION__, {}, fmt::format("error opening encoder ({})", unpackAVError(ret))));
    }

    // Copy encoder parameter to stream
    ret = avcodec_parameters_from_context(avs->codecpar, avcc);
    if (ret < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {},
                                                     fmt::format("error copying encoder parameters to the stream ({})",
                                                                 unpackAVError(ret))));
    }
}