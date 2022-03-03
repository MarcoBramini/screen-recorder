#ifndef PDS_SCREEN_RECORDING_FILTER_RING_H
#define PDS_SCREEN_RECORDING_FILTER_RING_H

#include "encoder_ring.h"
#include "process_context.h"
#include <variant>

extern "C" {
#include <libavformat/avformat.h>
}

class FilterChainRing {
    std::variant<std::shared_ptr<FilterChainRing>, std::shared_ptr<EncoderChainRing>> next;

public:
    virtual void execute(ProcessContext *processContext, AVFrame *inputFrame) = 0;

    std::variant<std::shared_ptr<FilterChainRing>, std::shared_ptr<EncoderChainRing>> getNext() { return this->next; };

    void setNext(std::variant<std::shared_ptr<FilterChainRing>, std::shared_ptr<EncoderChainRing>> ring) {
        this->next = std::move(ring);
    };

    virtual ~FilterChainRing() = default;
};

#endif //PDS_SCREEN_RECORDING_FILTER_RING_H
