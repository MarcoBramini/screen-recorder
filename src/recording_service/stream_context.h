#ifndef PDS_SCREEN_RECORDING_STREAM_CONTEXT_H
#define PDS_SCREEN_RECORDING_STREAM_CONTEXT_H

extern "C" {
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
};

#include <map>
#include <string>

class StreamContext {
    AVMediaType type;
    AVStream *avs;

public:
    static StreamContext from_input_stream(AVStream *stream);

    static StreamContext new_output_stream(AVFormatContext *avfc);

    AVStream *getStream() { return this->avs; };
};


#endif //PDS_SCREEN_RECORDING_STREAM_CONTEXT_H
