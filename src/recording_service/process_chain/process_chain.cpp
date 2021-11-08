#include "process_chain.h"

#include <utility>

void ProcessChain::processNext() {
    if (sourceQueue.empty()) {
        return;
    }

    // Get first packet
    ProcessContext *inputPacket = sourceQueue.front();
    sourceQueue.pop();

    decoderRing->execute(inputPacket);
    delete inputPacket;
}

ProcessChain::ProcessChain(DecoderChainRing *decoderRing, std::vector<FilterChainRing *> filterRings,
                           EncoderChainRing *encoderRing, MuxerChainRing *muxerRing) : decoderRing(decoderRing),
                                                                                       filterRings(
                                                                                               std::move(filterRings)),
                                                                                       encoderRing(encoderRing),
                                                                                       muxerRing(muxerRing) {
    // Set ring nexts
    if (this->filterRings.empty()) {
        this->decoderRing->setNext(this->encoderRing);
    } else {
        this->decoderRing->setNext(this->filterRings.front());

        int i;
        for (i = 0; i < this->filterRings.size() - 1; i++) {
            this->filterRings[i]->setNext(this->filterRings[i + 1]);
        }
        this->filterRings.back()->setNext(this->encoderRing);
    }
    this->encoderRing->setNext(this->muxerRing);
}

void ProcessChain::enqueueSourcePacket(AVPacket *p, int64_t pts) {
    auto *processContext = new ProcessContext(p, pts);
    sourceQueue.push(processContext);
}

void ProcessChain::flush() {
    // TODO: Flush everything else
    encoderRing->flush();
}

