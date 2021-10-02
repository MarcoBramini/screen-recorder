#ifndef PDS_SCREEN_RECORDING_INPUT_STREAMING_CONTEXT_H
#define PDS_SCREEN_RECORDING_INPUT_STREAMING_CONTEXT_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

/// Contains the information about one input device.
/// At least one between video and audio properties must be used.
/// If input device only support video/audio data, only video/audio properties will be set.
class InputStreamingContext {
    AVFormatContext *avfc;

    // Video properties
    const AVCodec *videoAvc;
    AVStream *videoAvs;
    AVCodecContext *videoAvcc;
    int videoIndex;

    // Audio properties
    const AVCodec *audioAvc;
    AVStream *audioAvs;
    AVCodecContext *audioAvcc;
    int audioIndex;
};

#endif //PDS_SCREEN_RECORDING_INPUT_STREAMING_CONTEXT_H
