#include "muxer_ring.h"
#include <fmt/core.h>

#include <utility>
#include "../error.h"

/// Initializes the muxer
MuxerChainRing::MuxerChainRing(std::shared_ptr<DeviceContext> muxerContext) : muxerContext(std::move(muxerContext)) {}

/// Processes an input encoded packet and writes it to the output
void MuxerChainRing::execute(ProcessContext *processContext, AVPacket *inputPacket) {
    int ret = av_interleaved_write_frame(muxerContext->getContext(), inputPacket);
    if (ret < 0) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {},
                                           fmt::format("error muxing the packet ({})", Error::unpackAVError(ret))));
    }
}

