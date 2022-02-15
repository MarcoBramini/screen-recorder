#ifndef PDS_SCREEN_RECORDING_FILTER_RING_H
#define PDS_SCREEN_RECORDING_FILTER_RING_H

#include "encoder_ring.h"
#include "process_context.h"
#include <variant>

extern "C" {
#include <libavformat/avformat.h>
};

class FilterChainRing {
    std::variant<FilterChainRing *, EncoderChainRing *> next;

public:
    virtual void execute(ProcessContext* processContext, AVFrame *inputFrame) = 0;

    std::variant<FilterChainRing *, EncoderChainRing *> getNext() { return this->next; };

    void setNext(std::variant<FilterChainRing *, EncoderChainRing *> ring) { this->next = ring; };
};

#endif //PDS_SCREEN_RECORDING_FILTER_RING_H
