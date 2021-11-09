
#ifndef PDS_SCREEN_RECORDING_SWRESAMPLE_FILTER_RING_H
#define PDS_SCREEN_RECORDING_SWRESAMPLE_FILTER_RING_H


#include "filter_ring.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
};

struct SWResampleConfig {
    int inputChannels;
    int64_t inputChannelLayout;
    AVSampleFormat inputSampleFormat;
    int inputSampleRate;
    int inputFrameSize;
    AVRational inputTimeBase;
    int outputChannels;
    int64_t outputChannelLayout;
    AVSampleFormat outputSampleFormat;
    int outputSampleRate;
    int outputFrameSize;
    AVRational outputTimeBase;
};

class SWResampleFilterRing : public FilterChainRing {
    SwrContext *swrContext;
    SWResampleConfig config;
    AVAudioFifo *outputBuffer;
public:
    explicit SWResampleFilterRing(SWResampleConfig config);

    ~SWResampleFilterRing() { swr_free(&swrContext); }

    void execute(ProcessContext *processContext, AVFrame *inputFrame) override;
};


#endif //PDS_SCREEN_RECORDING_SWRESAMPLE_FILTER_RING_H
