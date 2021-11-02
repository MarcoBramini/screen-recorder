#include "decoder_ring.h"
#include "../error.h"
#include <fmt/core.h>
#include "../stream_context.h"

/// Initializes the decoder for the current stream.
DecoderChainRing::DecoderChainRing(StreamContext inputStream) {
    streamCodec = avcodec_find_decoder(inputStream.getStream()->codecpar->codec_id);
    if (!streamCodec) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "no decoder found for the input stream"));
    }

    decoderContext = avcodec_alloc_context3(streamCodec);
    if (!decoderContext) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error allocating context for the decoder"));
    }

    int ret;
    ret = avcodec_parameters_to_context(decoderContext, inputStream.getStream()->codecpar);
    if (ret < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {},
                                                     fmt::format("error copying decoder parameters to the context ({})",
                                                                 unpackAVError(ret))));
    }

    ret = avcodec_open2(decoderContext, streamCodec, nullptr);
    if (ret < 0) {
        throw std::runtime_error(
                build_error_message(__FUNCTION__, {}, fmt::format("error opening decoder ({})", unpackAVError(ret))));
    }
}

void DecoderChainRing::execute(AVPacket *inputPacket) {
    AVFrame *decodedFrame = av_frame_alloc();
    if (decodedFrame == nullptr) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error allocating frame"));
    }

    int response = avcodec_send_packet(decoderContext, inputPacket);
    if (response < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error sending packet to the decoder"));
    }

    while (response >= 0) {
        response = avcodec_receive_frame(decoderContext, decodedFrame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            break;
        } else if (response < 0) {
            throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error receiving packet from the decoder"));
        }

        if (response >= 0) {
            if (std::holds_alternative<FilterChainRing *>(next)) {
                std::get<FilterChainRing *>(next)->execute(decodedFrame);
            } else {
                std::get<EncoderChainRing *>(next)->execute(decodedFrame);
            }
        }
    }
    av_frame_unref(decodedFrame);
}
