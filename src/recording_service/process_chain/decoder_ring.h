#ifndef PDS_SCREEN_RECORDING_DECODERING_H
#define PDS_SCREEN_RECORDING_DECODERING_H

#include "process_ring.h"
#include "encoder_ring.h"
#include "filter_ring.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
};

class DecoderChainRing {
    AVCodec *streamCodec;
    AVCodecContext *decoderContext;

    std::variant<FilterChainRing *, EncoderChainRing *> next;

public:
    explicit DecoderChainRing(StreamContext inputStream);

    void execute(AVPacket *inputPacket);

    void setNext(std::variant<FilterChainRing *, EncoderChainRing *> ring) { this->next = ring; };
};


#endif //PDS_SCREEN_RECORDING_DECODERING_H
