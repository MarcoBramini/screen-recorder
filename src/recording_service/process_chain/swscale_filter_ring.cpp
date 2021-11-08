#include "swscale_filter_ring.h"
#include "../error.h"

SWScaleFilterRing::SWScaleFilterRing(SWScaleConfig swScaleConfig) : config(swScaleConfig) {
    swsContext = sws_getContext(config.inputWidth, config.inputHeight, config.inputPixelFormat, config.outputWidth,
                                config.outputHeight, config.outputPixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (!swsContext) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, "error initializing video converter"));
    }
}

void SWScaleFilterRing::execute(ProcessContext* processContext, AVFrame *inputFrame) {
    AVFrame *convertedFrame = av_frame_alloc();

    convertedFrame->format = config.outputPixelFormat;
    convertedFrame->width = config.outputWidth;
    convertedFrame->height = config.outputHeight;
    int ret = av_frame_get_buffer(convertedFrame, 0);
    if (ret < 0 ){
        throw "aaaa";
    }
    if (sws_scale(swsContext, inputFrame->data, inputFrame->linesize, 0, inputFrame->height,
                  convertedFrame->data, convertedFrame->linesize) < 0) {
        throw std::runtime_error("failed converting video frame");
    }

    if (std::holds_alternative<FilterChainRing *>(getNext())) {
        std::get<FilterChainRing *>(getNext())->execute(processContext, convertedFrame);
    } else {
        std::get<EncoderChainRing *>(getNext())->execute(processContext, convertedFrame);
    }

    av_frame_free(&convertedFrame);
}