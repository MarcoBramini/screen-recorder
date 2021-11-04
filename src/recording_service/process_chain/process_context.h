//
// Created by Marco Bramini on 03/11/21.
//

#ifndef PDS_SCREEN_RECORDING_PROCESS_CONTEXT_H
#define PDS_SCREEN_RECORDING_PROCESS_CONTEXT_H

extern "C" {
#include <libavformat/avformat.h>
};

class ProcessContext {

public:
    AVPacket *sourcePacket;
    int64_t sourcePacketPts;

    ProcessContext(AVPacket *pkt, int64_t pts) : sourcePacket(pkt), sourcePacketPts(pts) {};

    ~ProcessContext() {
        av_packet_free(&sourcePacket);
    };
};


#endif //PDS_SCREEN_RECORDING_PROCESS_CONTEXT_H
