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

    std::queue<ProcessContext *> sourceQueue;

    DecoderChainRing *decoderRing;

    std::vector<FilterChainRing *> filterRings;

    EncoderChainRing *encoderRing;

    MuxerChainRing *muxerRing;

    bool isMainProcess; // video processing chain is considered the main process, audio is optional

public:
    ProcessChain(DecoderChainRing *decoderRing, std::vector<FilterChainRing *> filterRings,
                 EncoderChainRing *encoderRing, MuxerChainRing *muxerRing, bool isMainProcess);

    void processNext();

    void enqueueSourcePacket(AVPacket *p, int64_t pts);

    bool isSourceQueueEmpty() { return this->sourceQueue.empty(); };

    void flush();

    ~ProcessChain() {
        delete decoderRing;

        for (auto ring: filterRings) {
            delete ring;
        }

        delete encoderRing;

        if (isMainProcess) {
            delete muxerRing;
        }
    };
};


#endif //PDS_SCREEN_RECORDING_PROCESS_CHAIN_H
