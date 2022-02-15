#include "vfcrop_filter_ring.h"

#include <fmt/core.h>

#include "../error.h"

VFCropFilterRing::VFCropFilterRing(VFCropConfig config) : config(config) {
  std::string filter_descr =
      fmt::format("crop={}:{}:{}:{}", config.outputWidth, config.outputHeight,
                  config.originX, config.originY);
  char args[512];
  int ret = 0;
  const AVFilter* buffersrc = avfilter_get_by_name("buffer");
  const AVFilter* buffersink = avfilter_get_by_name("buffersink");
  AVFilterInOut* outputs = avfilter_inout_alloc();
  AVFilterInOut* inputs = avfilter_inout_alloc();
  enum AVPixelFormat pix_fmts[] = {config.outputPixelFormat, AV_PIX_FMT_NONE};

  filter_graph = avfilter_graph_alloc();
  if (!outputs || !inputs || !filter_graph) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {},
        fmt::format("error initializing filter graph ({})",
                    Error::unpackAVError(AVERROR(ENOMEM)))));
  }

  /* buffer video source: the decoded frames from the decoder will be inserted
   * here. */
  snprintf(args, sizeof(args),
           "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
           config.inputWidth, config.inputHeight, config.inputPixelFormat,
           config.inputTimeBase.num, config.inputTimeBase.den,
           config.inputAspectRatio.num, config.inputAspectRatio.den);

  ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args,
                                     NULL, filter_graph);
  if (ret < 0) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {},
        fmt::format("error creating buffer source ({})",
                    Error::unpackAVError(ret))));
  }

  /* buffer video sink: to terminate the filter chain. */
  ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL,
                                     NULL, filter_graph);
  if (ret < 0) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {},
        fmt::format("error creating buffer sink ({})",
                    Error::unpackAVError(ret))));
  }

  ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts,
                            AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
  if (ret < 0) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {},
        fmt::format("error setting output pixel format ({})",
                    Error::unpackAVError(ret))));
  }

  /*
   * Set the endpoints for the filter graph. The filter_graph will
   * be linked to the graph described by filters_descr.
   */

  /*
   * The buffer source output must be connected to the input pad of
   * the first filter described by filters_descr; since the first
   * filter input label is not specified, it is set to "in" by
   * default.
   */
  outputs->name = av_strdup("in");
  outputs->filter_ctx = buffersrc_ctx;
  outputs->pad_idx = 0;
  outputs->next = NULL;

  /*
   * The buffer sink input must be connected to the output pad of
   * the last filter described by filters_descr; since the last
   * filter output label is not specified, it is set to "out" by
   * default.
   */
  inputs->name = av_strdup("out");
  inputs->filter_ctx = buffersink_ctx;
  inputs->pad_idx = 0;
  inputs->next = NULL;

  if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_descr.c_str(),
                                      &inputs, &outputs, NULL)) < 0)
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {},
        fmt::format("error parsing the filter graph description ({})",
                    Error::unpackAVError(ret))));

  if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {},
        fmt::format("error configuring the filter graph ({})",
                    Error::unpackAVError(ret))));
}

void VFCropFilterRing::execute(ProcessContext* processContext,
                               AVFrame* inputFrame) {
  int ret = 0;
  AVFrame* convertedFrame = av_frame_alloc();

  /* push the decoded frame into the filtergraph */
  if (av_buffersrc_add_frame_flags(buffersrc_ctx, inputFrame,
                                   AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
    av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
  }

  /* pull filtered frames from the filtergraph */
  while (1) {
    ret = av_buffersink_get_frame(buffersink_ctx, convertedFrame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      break;
    if (ret < 0)
      throw std::runtime_error("error cropping video");
  }

  if (std::holds_alternative<FilterChainRing*>(getNext())) {
    std::get<FilterChainRing*>(getNext())->execute(processContext,
                                                   convertedFrame);
  } else {
    std::get<EncoderChainRing*>(getNext())->execute(processContext,
                                                    convertedFrame);
  }

  av_frame_free(&convertedFrame);
}