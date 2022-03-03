#ifndef PDS_SCREEN_RECORDING_VFCROP_FILTER_RING_H
#define PDS_SCREEN_RECORDING_VFCROP_FILTER_RING_H

#include "filter_ring.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

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

    std::unique_ptr<AVFilterGraph, FFMpegObjectsDeleter> filterGraph;
    // These are just convenience pointers to filter graph's filters contexts. They follow the filter graph lifecycle.
    AVFilterContext *bufferSinkCtx;
    AVFilterContext *bufferSrcCtx;

public:
    explicit VFCropFilterRing(VFCropConfig config);

    ~VFCropFilterRing() override = default;

    void execute(ProcessContext *processContext, AVFrame *inputFrame) override;
};

#endif  // PDS_SCREEN_RECORDING_VFCROP_FILTER_RING_H