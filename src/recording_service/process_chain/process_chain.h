#ifndef PDS_SCREEN_RECORDING_PROCESS_CHAIN_H
#define PDS_SCREEN_RECORDING_PROCESS_CHAIN_H


#include <queue>
#include <thread>

#include "../stream_context.h"
#include "process_ring.h"
#include "decoder_ring.h"
#include "filter_ring.h"
#include "encoder_ring.h"
#include "muxer_ring.h"


/// A process chain is a sequence of processes which starts from an AVPacket and finishes with a muxing operation into
/// an output file.
/// It takes an AVPacket queue in input.
class ProcessChain {

    std::queue<std::tuple<AVPacket *, int64_t>> sourceQueue;

    DecoderChainRing decoderRing;

    std::vector<FilterChainRing> filterRings;

    EncoderChainRing encoderRing;

    MuxerChainRing muxerRing;

public:
    ProcessChain(const DecoderChainRing &decoderRing, std::vector<FilterChainRing> &filterRings,
                 const EncoderChainRing &encoderRing, const MuxerChainRing &muxerRing);

    void processNext();
};


#endif //PDS_SCREEN_RECORDING_PROCESS_CHAIN_H
