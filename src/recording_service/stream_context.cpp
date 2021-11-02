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

    return streamContext;
}

/// Allocates a new stream for the provided device and returns a newly created StreamContext.
StreamContext StreamContext::new_output_stream(AVFormatContext *avfc) {
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

    return streamContext;
}

