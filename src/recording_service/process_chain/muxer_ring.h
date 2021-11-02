#ifndef PDS_SCREEN_RECORDING_MUXER_RING_H
#define PDS_SCREEN_RECORDING_MUXER_RING_H

#include "process_ring.h"

#include "../device_context.h"

extern "C" {
#include <libavformat/avformat.h>
};

class MuxerChainRing {
    DeviceContext muxerContext;

public:
    explicit MuxerChainRing(DeviceContext muxerContext);

    void execute(AVPacket *inputPacket);
};


#endif //PDS_SCREEN_RECORDING_MUXER_RING_H
