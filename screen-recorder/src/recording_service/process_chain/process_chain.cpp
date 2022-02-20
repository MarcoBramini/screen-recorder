#include "process_chain.h"

#include <utility>

/// Processes the next packet in the packets queue
void ProcessChain::processNext() {
    if (sourceQueue.empty()) {
        return;
    }

    auto inputPacket = std::move(sourceQueue.front());
    sourceQueue.pop();

    decoderRing->execute(inputPacket.get());
}

/// Initializes the process chain using the rings passed in input
ProcessChain::ProcessChain(std::shared_ptr<DecoderChainRing> decoderRing,
                           std::vector<std::shared_ptr<FilterChainRing>> filterRings,
                           std::shared_ptr<EncoderChainRing> encoderRing, std::shared_ptr<MuxerChainRing> muxerRing)
        : decoderRing(std::move(decoderRing)),
          filterRings(std::move(filterRings)),
          encoderRing(std::move(encoderRing)),
          muxerRing(std::move(muxerRing)) {
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
void ProcessChain::enqueueSourcePacket(std::unique_ptr<AVPacket, FFMpegObjectsDeleter> p, int64_t pts) {
    sourceQueue.emplace(std::make_unique<ProcessContext>(std::move(p), pts));
}

/// Flushes the whole chain stream
void ProcessChain::flush() {
    encoderRing->flush();
}

