#ifndef PDS_SCREEN_RECORDING_OUTPUT_STREAMING_CONTEXT_H
#define PDS_SCREEN_RECORDING_OUTPUT_STREAMING_CONTEXT_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

#include <string>

class OutputStreamingContext {
    AVFormatContext *avfc;

    const AVCodec *videoAvc;
    AVStream *videoAvs;
    AVCodecContext *videoAvcc;

    const AVCodec *audioAvc;
    AVStream *audioAvs;
    AVCodecContext *audioAvcc;

    const std::string filename;

public:
    std::string getFilename(){return this->filename;};
};

#endif //PDS_SCREEN_RECORDING_OUTPUT_STREAMING_CONTEXT_H
