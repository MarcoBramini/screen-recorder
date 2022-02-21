#ifndef PDS_SCREEN_RECORDING_ENCODER_RING_H
#define PDS_SCREEN_RECORDING_ENCODER_RING_H

#include "muxer_ring.h"
#include "../ffmpeg_objects_deleter.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
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
    // These are just convenience pointers to the context main A/V streams. They follow the context lifecycle.
    AVStream *inputStream;
    AVStream *outputStream;

    std::unique_ptr<AVCodecContext, FFMpegObjectsDeleter> encoderContext;

    int64_t lastEncodedDTS;

    std::shared_ptr<MuxerChainRing> next;

public:
    EncoderChainRing(AVStream *inputStream,
                     AVStream *outputStream,
                     const EncoderConfig &config);

    void execute(ProcessContext *processContext, AVFrame *inputFrame);

    void setNext(std::shared_ptr<MuxerChainRing> ring) { this->next = std::move(ring); };

    AVCodecContext *getEncoderContext() { return this->encoderContext.get(); };

    void flush();

    ~EncoderChainRing() = default;
};

#endif  // PDS_SCREEN_RECORDING_ENCODER_RING_H
