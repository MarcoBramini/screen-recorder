#ifndef PDS_SCREEN_RECORDING_ENCODER_RING_H
#define PDS_SCREEN_RECORDING_ENCODER_RING_H

#include "muxer_ring.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

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
    AVStream *inputStream;
    AVStream *outputStream;

    AVCodec *outputStreamCodec{};
    AVCodecContext *encoderContext;

    int64_t lastEncodedDTS;

    MuxerChainRing *next;

    void init_encoder(const EncoderConfig &config);

public:
    EncoderChainRing(AVStream *inputStream, AVStream *outputStream, const EncoderConfig &config);

    void execute(ProcessContext *processContext, AVFrame *inputFrame);

    void setNext(MuxerChainRing *ring) { this->next = ring; };

    AVCodecContext *getEncoderContext() { return this->encoderContext; };

    void flush();

    ~EncoderChainRing() {
        avcodec_close(encoderContext);
        avcodec_free_context(&encoderContext);
    };
};


#endif //PDS_SCREEN_RECORDING_ENCODER_RING_H
