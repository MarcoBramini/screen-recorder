#include "decoder_ring.h"
#include <fmt/core.h>
#include "../error.h"

/// Initializes the decoder for the current stream.
DecoderChainRing::DecoderChainRing(AVStream* inputStream) {
  auto streamCodec = avcodec_find_decoder(inputStream->codecpar->codec_id);
  if (!streamCodec) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {}, "no decoder found for the input stream"));
  }

  decoderContext = std::unique_ptr<AVCodecContext, FFMpegObjectsDeleter>(
      avcodec_alloc_context3(streamCodec));
  if (!decoderContext) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {}, "error allocating context for the decoder"));
  }

  int ret;
  ret = avcodec_parameters_to_context(decoderContext.get(),
                                      inputStream->codecpar);
  if (ret < 0) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {},
        fmt::format("error copying decoder parameters to the context ({})",
                    Error::unpackAVError(ret))));
  }

  ret = avcodec_open2(decoderContext.get(), streamCodec, nullptr);
  if (ret < 0) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {},
        fmt::format("error opening decoder ({})", Error::unpackAVError(ret))));
  }
}

/// Processes an input packet and passes the decoded frame to the next ring.
void DecoderChainRing::execute(ProcessContext* processContext) {
  auto decodedFrame =
      std::unique_ptr<AVFrame, FFMpegObjectsDeleter>(av_frame_alloc());
  if (decodedFrame == nullptr) {
    throw std::runtime_error(
        Error::build_error_message(__FUNCTION__, {}, "error allocating frame"));
  }

  int response = avcodec_send_packet(decoderContext.get(),
                                     processContext->sourcePacket.get());
  if (response < 0) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {},
        fmt::format("error sending packet to the decoder ({})",
                    Error::unpackAVError(response))));
  }

  while (response >= 0) {
    response = avcodec_receive_frame(decoderContext.get(), decodedFrame.get());
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    } else if (response < 0) {
      throw std::runtime_error(Error::build_error_message(
          __FUNCTION__, {},
          fmt::format("error receiving packet from the decoder ({})",
                      Error::unpackAVError(response))));
    }

    // Pass the decoded frame to the next ring
    if (response >= 0) {
      if (std::holds_alternative<std::shared_ptr<FilterChainRing>>(next)) {
        std::get<std::shared_ptr<FilterChainRing>>(next)->execute(
            processContext, decodedFrame.get());
      } else {
        std::get<std::shared_ptr<EncoderChainRing>>(next)->execute(
            processContext, decodedFrame.get());
      }
    }
  }
}
