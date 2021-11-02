//
// Created by Marco Bramini on 01/11/21.
//

#ifndef PDS_SCREEN_RECORDING_PROCESS_CHAIN_H
#define PDS_SCREEN_RECORDING_PROCESS_CHAIN_H


#include "stream_context.h"

class ProcessRing {

public:
    virtual void execute() = 0;
};


class DecodeRing : private ProcessRing {
    StreamContext *streamContext;
    AVCodecContext avcc;

public:
    void execute(AVPacket *inputPacket, AVFrame *outputFrame);
};

class ScaleFilterRing : private ProcessRing {
    StreamContext *streamContext;
    SWContext swContext;
public:
    void execute(AVFrame *inputFrame, AVFrame *outputFrame);
};


#endif //PDS_SCREEN_RECORDING_PROCESS_CHAIN_H
