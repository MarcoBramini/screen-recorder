#include "swscale_filter_ring.h"
#include <fmt/core.h>
#include "../error.h"

/// Initializes a scale filter, used to scale an input video decoded frame to the output format
SWScaleFilterRing::SWScaleFilterRing(SWScaleConfig swScaleConfig)
        : config(swScaleConfig) {
    swsContext = std::unique_ptr<SwsContext, FFMpegObjectsDeleter>(sws_getContext(config.inputWidth, config.inputHeight,
                                                                                  config.inputPixelFormat,
                                                                                  config.outputWidth,
                                                                                  config.outputHeight,
                                                                                  config.outputPixelFormat,
                                                                                  SWS_BICUBIC, nullptr, nullptr,
                                                                                  nullptr));
    if (!swsContext) {
        throw std::runtime_error(Error::build_error_message(
                __FUNCTION__, {}, "error initializing video converter"));
    }
}

/// Processes an input frame and passes it to the next ring
void SWScaleFilterRing::execute(ProcessContext *processContext, AVFrame *inputFrame) {
    auto convertedFrame = std::unique_ptr<AVFrame, FFMpegObjectsDeleter>(av_frame_alloc());
    if (!convertedFrame) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, "error allocating a new frame"));
    }

    convertedFrame->format = config.outputPixelFormat;
    convertedFrame->width = config.outputWidth;
    convertedFrame->height = config.outputHeight;

    int ret = av_frame_get_buffer(convertedFrame.get(), 0);
    if (ret < 0) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {},
                                           fmt::format("error allocating the converted frame video buffer ({})",
                                                       Error::unpackAVError(ret))));
    }

    ret = sws_scale(swsContext.get(), inputFrame->data, inputFrame->linesize, 0,
                    inputFrame->height, convertedFrame->data, convertedFrame->linesize);
    if (ret < 0) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {},
                                           fmt::format("error converting the input video frame ({})",
                                                       Error::unpackAVError(ret))));
    }

    // Pass the converted frame to the next ring
    if (std::holds_alternative<std::shared_ptr<FilterChainRing>>(getNext())) {
        std::get<std::shared_ptr<FilterChainRing>>(getNext())->execute(processContext,
                                                                       convertedFrame.get());
    } else {
        std::get<std::shared_ptr<EncoderChainRing>>(getNext())->execute(processContext,
                                                                        convertedFrame.get());
    }
}