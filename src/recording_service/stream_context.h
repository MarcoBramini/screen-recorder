#ifndef PDS_SCREEN_RECORDING_STREAM_CONTEXT_H
#define PDS_SCREEN_RECORDING_STREAM_CONTEXT_H

extern "C" {
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
};

#include <map>
#include <string>

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
    int channelLayout;
    int sampleRate;
    AVSampleFormat sampleFormat;
    int strictStdCompliance;
};

class StreamContext {
    AVMediaType type;
    AVStream *avs;
    AVCodec *avc;
    AVCodecContext *avcc;

    void init_decoder();

    void init_encoder(const EncoderConfig &config);

public:
    static StreamContext from_input_stream(AVStream *stream);

    static StreamContext new_output_stream(AVFormatContext *avfc, const EncoderConfig &config);

};


#endif //PDS_SCREEN_RECORDING_STREAM_CONTEXT_H
