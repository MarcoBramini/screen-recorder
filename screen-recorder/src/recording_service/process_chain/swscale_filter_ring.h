#ifndef PDS_SCREEN_RECORDING_SWSCALE_FILTER_RING_H
#define PDS_SCREEN_RECORDING_SWSCALE_FILTER_RING_H

#include "filter_ring.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
};

struct SWScaleConfig {
    int inputWidth;
    int inputHeight;
    AVPixelFormat inputPixelFormat;
    int outputWidth;
    int outputHeight;
    AVPixelFormat outputPixelFormat;
};

class SWScaleFilterRing : public FilterChainRing {
    SwsContext *swsContext;
    SWScaleConfig config;

public:
    explicit SWScaleFilterRing(SWScaleConfig config);

    ~SWScaleFilterRing() { sws_freeContext(swsContext); }

    void execute(ProcessContext* processContext, AVFrame *inputFrame) override;
};


#endif //PDS_SCREEN_RECORDING_SWSCALE_FILTER_RING_H
