#include "vfcrop_filter_ring.h"
#include <fmt/core.h>
#include "../error.h"

/// Initializes a scale filter, used to scale an input video decoded frame to the output format
VFCropFilterRing::VFCropFilterRing(VFCropConfig config) : config(config) {
    std::string filter_descr =
            fmt::format("crop={}:{}:{}:{}", config.outputWidth, config.outputHeight,
                        config.originX, config.originY);

    char args[512];
    int ret;

    const AVFilter *bufferSrc = avfilter_get_by_name("buffer");
    const AVFilter *bufferSink = avfilter_get_by_name("buffersink");

    auto outputs = avfilter_inout_alloc(); // WARN A smart pointer cannot be immediately used because of the avfilter_graph_parse_ptr function which requires a pointer of pointer
    auto inputs = avfilter_inout_alloc(); // WARN A smart pointer cannot be immediately used because of the avfilter_graph_parse_ptr function which requires a pointer of pointer
    filterGraph = std::unique_ptr<AVFilterGraph, FFMpegObjectsDeleter>(avfilter_graph_alloc());
    if (!outputs || !inputs || !filterGraph) {
        throw std::runtime_error(Error::build_error_message(__FUNCTION__, {},
                                                            fmt::format("error initializing filter graph ({})",
                                                                        Error::unpackAVError(AVERROR(ENOMEM)))));
    }

    // Init the buffer video source: the decoded frames from the decoder will be inserted here
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             config.inputWidth, config.inputHeight, config.inputPixelFormat,
             config.inputTimeBase.num, config.inputTimeBase.den,
             config.inputAspectRatio.num, config.inputAspectRatio.den);

    bufferSrcCtx = nullptr;
    ret = avfilter_graph_create_filter(&bufferSrcCtx, bufferSrc, "in", args,
                                       nullptr, filterGraph.get());
    if (ret < 0) {
        throw std::runtime_error(Error::build_error_message(
                __FUNCTION__, {},
                fmt::format("error creating buffer source ({})",
                            Error::unpackAVError(ret))));
    }

    // Init the buffer video sink: filter chain termination, emits the converted frames.
    bufferSinkCtx = nullptr;
    ret = avfilter_graph_create_filter(&bufferSinkCtx, bufferSink, "out", nullptr,
                                       nullptr, filterGraph.get());
    if (ret < 0) {
        throw std::runtime_error(Error::build_error_message(
                __FUNCTION__, {},
                fmt::format("error creating buffer sink ({})",
                            Error::unpackAVError(ret))));
    }

    enum AVPixelFormat pix_fmts[] = {config.outputPixelFormat, AV_PIX_FMT_NONE};
    ret = av_opt_set_int_list(bufferSinkCtx, "pix_fmts", pix_fmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        throw std::runtime_error(Error::build_error_message(
                __FUNCTION__, {},
                fmt::format("error setting output pixel format ({})",
                            Error::unpackAVError(ret))));
    }

    /*
     * Set the endpoints for the filter graph. The filterGraph will
     * be linked to the graph described by filters_descr.
     */

    /*
     * The buffer source output must be connected to the input pad of
     * the first filter described by filters_descr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
     */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = bufferSrcCtx;
    outputs->pad_idx = 0;
    outputs->next = nullptr;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name = av_strdup("out");
    inputs->filter_ctx = bufferSinkCtx;
    inputs->pad_idx = 0;
    inputs->next = nullptr;

    if ((ret = avfilter_graph_parse_ptr(filterGraph.get(), filter_descr.c_str(),
                                        &inputs, &outputs, nullptr)) < 0)
        throw std::runtime_error(Error::build_error_message(
                __FUNCTION__, {},
                fmt::format("error parsing the filter graph description ({})",
                            Error::unpackAVError(ret))));

    inputs = std::unique_ptr<AVFilterInOut, FFMpegObjectsDeleter>(inputs).get();
    outputs = std::unique_ptr<AVFilterInOut, FFMpegObjectsDeleter>(outputs).get();

    if ((ret = avfilter_graph_config(filterGraph.get(), nullptr)) < 0)
        throw std::runtime_error(Error::build_error_message(
                __FUNCTION__, {},
                fmt::format("error configuring the filter graph ({})",
                            Error::unpackAVError(ret))));
}

void VFCropFilterRing::execute(ProcessContext *processContext, AVFrame *inputFrame) {
    int ret;
    auto convertedFrame = std::unique_ptr<AVFrame, FFMpegObjectsDeleter>(av_frame_alloc());
    if (!convertedFrame) {
        throw std::runtime_error(
                Error::build_error_message(__FUNCTION__, {}, "error allocating a new frame"));
    }

    // Send the input frame to the filtergraph
    ret = av_buffersrc_add_frame_flags(bufferSrcCtx, inputFrame,
                                       AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret < 0) {
        throw std::runtime_error(Error::build_error_message(__FUNCTION__, {},
                                                            fmt::format(
                                                                    "error sending the input frame to the filter graph ({})",
                                                                    Error::unpackAVError(ret))));
    }

    // Read the converted frames from the filtergraph
    while (true) {
        ret = av_buffersink_get_frame(bufferSinkCtx, convertedFrame.get());
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret < 0)
            throw std::runtime_error(Error::build_error_message(__FUNCTION__, {},
                                                                fmt::format(
                                                                        "error receiving the converted frame from the filter graph ({})",
                                                                        Error::unpackAVError(ret))));
    }

    // Pass the converted frame to the next ring
    if (std::holds_alternative<std::shared_ptr<FilterChainRing>>(getNext())) {
        std::get<std::shared_ptr<FilterChainRing>>(getNext())->execute(processContext, convertedFrame.get());
    } else {
        std::get<std::shared_ptr<EncoderChainRing>>(getNext())->execute(processContext, convertedFrame.get());
    }
}