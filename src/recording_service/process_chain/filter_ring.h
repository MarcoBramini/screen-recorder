//
// Created by Marco Bramini on 01/11/21.
//

#ifndef PDS_SCREEN_RECORDING_FILTER_RING_H
#define PDS_SCREEN_RECORDING_FILTER_RING_H

#include "process_ring.h"
#include "encoder_ring.h"

extern "C" {
#include <libavformat/avformat.h>
};

class FilterChainRing {

    std::variant<FilterChainRing *, EncoderChainRing *> next;

public:
    void execute(AVFrame *inputFrame);
    void setNext(std::variant<FilterChainRing *, EncoderChainRing *> ring) { this->next = ring; };
};


#endif //PDS_SCREEN_RECORDING_FILTER_RING_H
