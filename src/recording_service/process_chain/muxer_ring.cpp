#include "muxer_ring.h"
#include <fmt/core.h>
#include "../error.h"

MuxerChainRing::MuxerChainRing(DeviceContext muxerContext): muxerContext(muxerContext) {}

void MuxerChainRing::execute(AVPacket *inputPacket) {
    int ret = av_interleaved_write_frame(muxerContext.getContext(), inputPacket);
    if (ret < 0) {
        throw std::runtime_error(
                build_error_message(__FUNCTION__, {}, fmt::format("error muxing the packet ({})", unpackAVError(ret))));
    }
}
