#ifndef PDS_SCREEN_RECORDING_DECODERING_H
#define PDS_SCREEN_RECORDING_DECODERING_H

#include "encoder_ring.h"
#include "filter_ring.h"
#include "process_context.h"
#include "../ffmpeg_objects_deleter.h"
#include <variant>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

class DecoderChainRing {
    std::unique_ptr<AVCodecContext, FFMpegObjectsDeleter> decoderContext;

    std::variant<std::shared_ptr<FilterChainRing>, std::shared_ptr<EncoderChainRing>> next;

public:
    explicit DecoderChainRing(AVStream *inputStream);

    void execute(ProcessContext *processContext);

    void setNext(std::variant<std::shared_ptr<FilterChainRing>, std::shared_ptr<EncoderChainRing>> ring) {
        this->next = std::move(ring);
    };

    AVCodecContext *getDecoderContext() { return this->decoderContext.get(); };

    ~DecoderChainRing() = default;
};


#endif //PDS_SCREEN_RECORDING_DECODERING_H
