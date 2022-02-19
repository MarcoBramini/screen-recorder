#include "swscale_filter_ring.h"
#include <fmt/core.h>
#include "../error.h"

/// Initializes a scale filter, used to scale an input video decoded frame to the output format
SWScaleFilterRing::SWScaleFilterRing(SWScaleConfig swScaleConfig)
        : config(swScaleConfig) {
    swsContext = sws_getContext(config.inputWidth, config.inputHeight,
                                config.inputPixelFormat, config.outputWidth,
                                config.outputHeight, config.outputPixelFormat,
                                SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (!swsContext) {
        throw std::runtime_error(Error::build_error_message(
                __FUNCTION__, {}, "error initializing video converter"));
    }
}

/// Processes an input frame and passes it to the next ring
void SWScaleFilterRing::execute(ProcessContext *processContext,
                                AVFrame *inputFrame) {
    AVFrame *convertedFrame = av_frame_alloc();
    if (!convertedFrame) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, "error allocating a new frame"));
    }

    convertedFrame->format = config.outputPixelFormat;
    convertedFrame->width = config.outputWidth;
    convertedFrame->height = config.outputHeight;

    int ret = av_frame_get_buffer(convertedFrame, 0);
    if (ret < 0) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {},
                                           fmt::format("error allocating the converted frame video buffer ({})",
                                                       Error::unpackAVError(ret))));
    }

    ret = sws_scale(swsContext, inputFrame->data, inputFrame->linesize, 0,
                    inputFrame->height, convertedFrame->data, convertedFrame->linesize);
    if (ret < 0) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {},
                                           fmt::format("error converting the input video frame ({})",
                                                       Error::unpackAVError(ret))));
    }

    // Pass the converted frame to the next ring
    if (std::holds_alternative<FilterChainRing *>(getNext())) {
        std::get<FilterChainRing *>(getNext())->execute(processContext,
                                                        convertedFrame);
    } else {
        std::get<EncoderChainRing *>(getNext())->execute(processContext,
                                                         convertedFrame);
    }

    av_frame_free(&convertedFrame);
}