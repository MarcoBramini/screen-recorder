#include "process_chain.h"

#include <utility>

void ProcessChain::processNext() {
    if (sourceQueue.empty()) {
        return;
    }

    // Get first packet
    AVPacket *inputPacket;
    int64_t packetPts;
    std::tie(inputPacket, packetPts) = sourceQueue.front();
    sourceQueue.pop();

    decoderRing.execute(inputPacket);

    av_packet_unref(inputPacket);
    av_packet_free(&inputPacket);
}

ProcessChain::ProcessChain(const DecoderChainRing &decoderRing,
                           std::vector<FilterChainRing> &filterRings, const EncoderChainRing &encoderRing,
                           const MuxerChainRing &muxerRing) : decoderRing(decoderRing),
                                                              filterRings(filterRings), encoderRing(encoderRing),
                                                              muxerRing(muxerRing) {
    // Set ring nexts
    this->decoderRing.setNext(&this->filterRings.front());

    int i;
    for (i = 0; i < this->filterRings.size() - 1; i++) {
        this->filterRings[i].setNext(&this->filterRings[i + 1]);
    }
    this->filterRings.back().setNext(&this->encoderRing);

    this->encoderRing.setNext(&this->muxerRing);
}

