#ifndef PDS_SCREEN_RECORDING_PROCESS_CONTEXT_H
#define PDS_SCREEN_RECORDING_PROCESS_CONTEXT_H

#include "../ffmpeg_objects_deleter.h"

extern "C" {
#include <libavformat/avformat.h>
}

class ProcessContext {

public:
    std::unique_ptr<AVPacket, FFMpegObjectsDeleter> sourcePacket;
    int64_t sourcePacketPts;

    ProcessContext(std::unique_ptr<AVPacket, FFMpegObjectsDeleter> pkt, int64_t pts) : sourcePacket(std::move(pkt)),
                                                                                       sourcePacketPts(pts) {};

    ~ProcessContext() = default;
};


#endif //PDS_SCREEN_RECORDING_PROCESS_CONTEXT_H
