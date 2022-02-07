#ifndef PDS_SCREEN_RECORDING_MUXER_RING_H
#define PDS_SCREEN_RECORDING_MUXER_RING_H

#include "../device_context.h"
#include "process_context.h"

extern "C" {
#include <libavformat/avformat.h>
};

class MuxerChainRing {
    DeviceContext* muxerContext;

public:
    explicit MuxerChainRing(DeviceContext* muxerContext);

    void execute(ProcessContext* processContext, AVPacket* inputPacket);

    ~MuxerChainRing();
};


#endif //PDS_SCREEN_RECORDING_MUXER_RING_H
