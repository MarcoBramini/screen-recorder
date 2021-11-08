#include "decoder_ring.h"
#include "../error.h"
#include <fmt/core.h>

/// Initializes the decoder for the current stream.
DecoderChainRing::DecoderChainRing(AVStream *inputStream) {
    streamCodec = avcodec_find_decoder(inputStream->codecpar->codec_id);
    if (!streamCodec) {
        throw std::runtime_error(Error::build_error_message(__FUNCTION__, {}, "no decoder found for the input stream"));
    }

    decoderContext = avcodec_alloc_context3(streamCodec);
    if (!decoderContext) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, "error allocating context for the decoder"));
    }

    int ret;
    ret = avcodec_parameters_to_context(decoderContext, inputStream->codecpar);
    if (ret < 0) {
        throw std::runtime_error(Error::build_error_message(__FUNCTION__, {},
                                                            fmt::format(
                                                                    "error copying decoder parameters to the context ({})",
                                                                    Error::unpackAVError(ret))));
    }

    ret = avcodec_open2(decoderContext, streamCodec, nullptr);
    if (ret < 0) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {},
                                           fmt::format("error opening decoder ({})", Error::unpackAVError(ret))));
    }
}

void DecoderChainRing::execute(ProcessContext* processContext) {
    AVFrame *decodedFrame = av_frame_alloc();
    if (decodedFrame == nullptr) {
        throw std::runtime_error(Error::build_error_message(__FUNCTION__, {}, "error allocating frame"));
    }

    int response = avcodec_send_packet(decoderContext, processContext->sourcePacket);
    if (response < 0) {
        throw std::runtime_error(Error::build_error_message(__FUNCTION__, {}, "error sending packet to the decoder"));
    }

    while (response >= 0) {
        response = avcodec_receive_frame(decoderContext, decodedFrame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            break;
        } else if (response < 0) {
            throw std::runtime_error(
                    Error::build_error_message(__FUNCTION__, {}, "error receiving packet from the decoder"));
        }

        if (response >= 0) {
            if (std::holds_alternative<FilterChainRing *>(next)) {
                std::get<FilterChainRing *>(next)->execute(processContext, decodedFrame);
            } else {
                std::get<EncoderChainRing *>(next)->execute(processContext, decodedFrame);
            }
        }
    }
    av_frame_free(&decodedFrame);
}
