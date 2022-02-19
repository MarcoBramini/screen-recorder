#include "muxer_ring.h"
#include <fmt/core.h>
#include "../error.h"

/// Initializes the muxer
MuxerChainRing::MuxerChainRing(DeviceContext* muxerContext): muxerContext(muxerContext) {}

/// Processes an input encoded packet and writes it to the output
void MuxerChainRing::execute(ProcessContext* processContext, AVPacket* inputPacket) {
    int ret = av_interleaved_write_frame(muxerContext->getContext(), inputPacket);
    if (ret < 0) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, fmt::format("error muxing the packet ({})", Error::unpackAVError(ret))));
    }
}

