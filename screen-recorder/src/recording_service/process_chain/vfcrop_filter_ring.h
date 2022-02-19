#ifndef PDS_SCREEN_RECORDING_VFCROP_FILTER_RING_H
#define PDS_SCREEN_RECORDING_VFCROP_FILTER_RING_H

#include "filter_ring.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
};

struct VFCropConfig {
    int inputWidth;
    int inputHeight;
    AVPixelFormat inputPixelFormat;
    AVRational inputTimeBase;
    AVRational inputAspectRatio;

    int originX;
    int originY;

    int outputWidth;
    int outputHeight;
    AVPixelFormat outputPixelFormat;
};

class VFCropFilterRing : public FilterChainRing {
    VFCropConfig config;

    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;

public:
    explicit VFCropFilterRing(VFCropConfig config);

    ~VFCropFilterRing() override {
        avfilter_graph_free(&filter_graph);
        avfilter_free(buffersink_ctx);
        avfilter_free(buffersrc_ctx);
    }

    void execute(ProcessContext *processContext, AVFrame *inputFrame) override;
};

#endif  // PDS_SCREEN_RECORDING_VFCROP_FILTER_RING_H
