#ifndef PDS_SCREEN_RECORDING_DECODERING_H
#define PDS_SCREEN_RECORDING_DECODERING_H

#include "encoder_ring.h"
#include "filter_ring.h"
#include "process_context.h"
#include <variant>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

class DecoderChainRing {
    AVCodec *streamCodec;
    AVCodecContext *decoderContext;

    std::variant<FilterChainRing *, EncoderChainRing *> next;

public:
    explicit DecoderChainRing(AVStream *inputStream);

    void execute(ProcessContext *processContext);

    void setNext(std::variant<FilterChainRing *, EncoderChainRing *> ring) { this->next = ring; };

    AVCodecContext *getDecoderContext() { return this->decoderContext; };

    ~DecoderChainRing() {
        avcodec_close(decoderContext);
        avcodec_free_context(&decoderContext);
    };
};


#endif //PDS_SCREEN_RECORDING_DECODERING_H
