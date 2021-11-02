//
// Created by Marco Bramini on 01/11/21.
//

#ifndef PDS_SCREEN_RECORDING_ENCODER_RING_H
#define PDS_SCREEN_RECORDING_ENCODER_RING_H

#include "process_ring.h"
#include "../stream_context.h"
#include "muxer_ring.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
};

struct EncoderConfig {
    AVCodecID codecID;
    AVMediaType codecType;
    std::map<std::string, std::string> encoderOptions;
    int bitRate;

    // Video properties
    int height;
    int width;
    AVPixelFormat pixelFormat;
    int frameRate;

    // Audio properties
    int channels;
    int64_t channelLayout;
    int sampleRate;
    AVSampleFormat sampleFormat;
    int strictStdCompliance;
};

class EncoderChainRing {
    StreamContext inputStream;
    StreamContext outputStream;

    AVCodecContext *encoderContext;

    int64_t lastEncodedDTS;

    MuxerChainRing* next;

    void init_encoder(const EncoderConfig &config);

public:
    EncoderChainRing(StreamContext inputStream, StreamContext outputStream, const EncoderConfig &config);

    void execute(AVFrame *inputFrame);
    void setNext(MuxerChainRing* ring) { this->next = ring; };
};


#endif //PDS_SCREEN_RECORDING_ENCODER_RING_H
