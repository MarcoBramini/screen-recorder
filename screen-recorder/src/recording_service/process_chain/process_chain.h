#ifndef PDS_SCREEN_RECORDING_PROCESS_CHAIN_H
#define PDS_SCREEN_RECORDING_PROCESS_CHAIN_H

#include <queue>
#include <thread>
#include <iostream>

#include "decoder_ring.h"
#include "filter_ring.h"
#include "encoder_ring.h"
#include "muxer_ring.h"
#include "process_context.h"

/// A process chain is a sequence of processes which starts from an AVPacket and finishes with a muxing operation into
/// an output file.
/// It takes an AVPacket queue in input.
class ProcessChain {

    std::queue<std::unique_ptr<ProcessContext>> sourceQueue;

    std::shared_ptr<DecoderChainRing> decoderRing;

    std::vector<std::shared_ptr<FilterChainRing>> filterRings;

    std::shared_ptr<EncoderChainRing> encoderRing;

    std::shared_ptr<MuxerChainRing> muxerRing;

public:
    ProcessChain(std::shared_ptr<DecoderChainRing> decoderRing,
                 std::vector<std::shared_ptr<FilterChainRing>> filterRings,
                 std::shared_ptr<EncoderChainRing> encoderRing, std::shared_ptr<MuxerChainRing> muxerRing);

    void processNext();

    void enqueueSourcePacket(std::unique_ptr<AVPacket, FFMpegObjectsDeleter>, int64_t pts);

    [[nodiscard]] bool isSourceQueueEmpty() const { return this->sourceQueue.empty(); };

    void flush();

    ~ProcessChain() = default;
};


#endif //PDS_SCREEN_RECORDING_PROCESS_CHAIN_H
