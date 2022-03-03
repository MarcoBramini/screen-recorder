#ifndef SCREEN_RECORDER_FFMPEG_OBJECTS_DELETER_H
#define SCREEN_RECORDER_FFMPEG_OBJECTS_DELETER_H

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavutil/audio_fifo.h"
#include "libswscale/swscale.h"
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}

class FFMpegObjectsDeleter {

public:
    void operator()(AVCodecContext *avcc);

    void operator()(AVFormatContext *avfc);

    void operator()(AVFrame *avf);

    void operator()(AVPacket *avp);

    void operator()(SwrContext *swrc);

    void operator()(AVAudioFifo *avaf);

    void operator()(uint8_t ***d);

    void operator()(SwsContext *swsc);

    void operator()(AVFilterGraph *avfg);

    void operator()(AVFilterInOut *avfio);
};


#endif //SCREEN_RECORDER_FFMPEG_OBJECTS_DELETER_H
