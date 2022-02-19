#include "process_chain.h"

#include <utility>

/// Processes the next packet in the packets queue
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

/// Initializes the process chain using the rings passed in input.
ProcessChain::ProcessChain(DecoderChainRing *decoderRing,
                           std::vector<FilterChainRing *> filterRings,
                           EncoderChainRing *encoderRing,
                           MuxerChainRing *muxerRing,
                           bool isMainProcess) : decoderRing(decoderRing),
                                                 filterRings(std::move(filterRings)),
                                                 encoderRing(encoderRing),
                                                 muxerRing(muxerRing),
                                                 isMainProcess(isMainProcess) {
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

/// Enqueues a packet for processing
void ProcessChain::enqueueSourcePacket(AVPacket *p, int64_t pts) {
    auto *processContext = new ProcessContext(p, pts);
    sourceQueue.push(processContext);
}

/// Flushes the whole chain stream
void ProcessChain::flush() {
    encoderRing->flush();
}

