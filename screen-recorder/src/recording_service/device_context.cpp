#include "device_context.h"
#include <fmt/core.h>
#include "error.h"

/// Initializes the demuxer device identifier by the passed deviceID and A/V
/// urls. Device options can be set by populating the options map.
std::shared_ptr<DeviceContext> DeviceContext::init_demuxer(
    const std::string& deviceID,
    const std::string& videoURL,
    const std::string& audioURL,
    const std::map<std::string, std::string>& optionsMap) {
  auto deviceContext = std::make_shared<DeviceContext>();
  deviceContext->avfc =
      init_input_device(deviceID, videoURL, audioURL, optionsMap);

  if (!videoURL.empty()) {
    int videoStreamID = deviceContext->find_main_stream(AVMEDIA_TYPE_VIDEO);
    deviceContext->videoStream = deviceContext->avfc->streams[videoStreamID];
  }

  if (!audioURL.empty()) {
    int audioStreamID = deviceContext->find_main_stream(AVMEDIA_TYPE_AUDIO);
    deviceContext->audioStream = deviceContext->avfc->streams[audioStreamID];
  }
  return deviceContext;
}

/// Initializes the muxer device identifier.
/// The output path must be a full path and have .mp4 extension.
std::shared_ptr<DeviceContext> DeviceContext::init_muxer(
    const std::string& outputPath,
    bool isAudioDisabled) {
  // Build method params for error handling purposes
  std::map<std::string, std::string> methodParams = {
      {"outputPath", outputPath},
      {"isAudioDisabled", std::to_string(isAudioDisabled)}};

  auto deviceContext = std::make_shared<DeviceContext>();

  deviceContext->avfc = init_output_context(outputPath);

  // Add new output video stream
  deviceContext->videoStream =
      avformat_new_stream(deviceContext->avfc.get(), nullptr);
  if (!deviceContext->videoStream) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, methodParams, "error allocating output AVStream"));
  }
  deviceContext->videoStream->time_base = {1, 1000};

  if (!isAudioDisabled) {
    // Add new output audio stream
    deviceContext->audioStream =
        avformat_new_stream(deviceContext->avfc.get(), nullptr);
    if (!deviceContext->audioStream) {
      throw std::runtime_error(Error::build_error_message(
          __FUNCTION__, methodParams, "error allocating output AVStream"));
    }
    deviceContext->audioStream->time_base = {1, 1000};
  }
  return deviceContext;
}

/// Initializes the device identified by the passed deviceID and A/V urls.
std::unique_ptr<AVFormatContext, FFMpegObjectsDeleter>
DeviceContext::init_input_device(
    const std::string& deviceID,
    const std::string& videoURL,
    const std::string& audioURL,
    const std::map<std::string, std::string>& optionsMap) {
  // Build method params for error handling purposes
  std::map<std::string, std::string> methodParams = {
      {"deviceID", deviceID}, {"videoURL", videoURL}, {"audioURL", audioURL}};

  // Allocate a context for the device
  auto ctx =
      avformat_alloc_context();  // WARN A smart pointer cannot be immediately
                                 // used because of the avformat_open_input
                                 // function which requires a pointer of pointer
  if (!ctx) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, methodParams, "error during AVFormatContext allocation"));
  }

  // Find the input format associated to the deviceID
  auto inputFormat = av_find_input_format(deviceID.c_str());
  if (!inputFormat) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, methodParams, "no AVInputFormat found for deviceID"));
  }

  // Build the options for the device
  AVDictionary* options = nullptr;
  int ret;
  for (const auto& option : optionsMap) {
    ret = av_dict_set(&options, option.first.c_str(), option.second.c_str(), 0);
    if (ret < 0) {
      throw std::runtime_error(Error::build_error_message(
          __FUNCTION__, methodParams,
          fmt::format("error setting '{}' option to value '{}' ({})",
                      option.first, option.second, Error::unpackAVError(ret))));
    }
  }

  // Open the input device
  std::string url;
  if (!videoURL.empty() && audioURL.empty()) {
    url = videoURL;
  } else if (videoURL.empty() && !audioURL.empty()) {
    url = audioURL;
  } else {
    url = videoURL + ":" + audioURL;
  }
  ret = avformat_open_input(&ctx, url.c_str(), inputFormat, &options);
  if (ret < 0) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, methodParams,
        fmt::format("error opening input format ({})",
                    Error::unpackAVError(ret))));
  }

  // Find device's streams info
  ctx->probesize = 100000000;  // size of the buffer containing the frames used
                               // to get streams info
  ret = avformat_find_stream_info(ctx, nullptr);
  if (ret < 0) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, methodParams,
        fmt::format("error finding streams info ({})",
                    Error::unpackAVError(ret))));
  }

  return std::unique_ptr<AVFormatContext, FFMpegObjectsDeleter>(ctx);
}

/// Finds the main video stream in the current device context.
int DeviceContext::find_main_stream(AVMediaType streamType) {
  // Build method params for error handling purposes
  std::map<std::string, std::string> methodParams = {
      {"streamType", std::to_string(streamType)}};

  if (avfc == nullptr) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, methodParams, "device not initialized"));
  }

  int streamIndex =
      av_find_best_stream(avfc.get(), streamType, -1, -1, nullptr, 0);
  if (streamIndex < 0) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, methodParams,
        fmt::format("error finding main stream for type {}", streamType)));
  }

  return streamIndex;
}

/// Allocates the output context and open the output file.
std::unique_ptr<AVFormatContext, FFMpegObjectsDeleter>
DeviceContext::init_output_context(const std::string& outputPath) {
  // Build method params for error handling purposes
  std::map<std::string, std::string> methodParams = {
      {"outputPath", outputPath}};

  AVFormatContext* ctx;
  int ret = avformat_alloc_output_context2(&ctx, nullptr, nullptr,
                                           outputPath.c_str());
  if (ret < 0) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, methodParams,
        fmt::format("error during output AVFormatContext allocation ({})",
                    Error::unpackAVError(ret))));
  }

  if (ctx->oformat->flags & AVFMT_GLOBALHEADER)
    ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

  if (!(ctx->oformat->flags & AVFMT_NOFILE)) {
    ret = avio_open(&ctx->pb, outputPath.c_str(), AVIO_FLAG_WRITE);
    if (ret < 0) {
      throw std::runtime_error(Error::build_error_message(
          __FUNCTION__, methodParams,
          fmt::format("error opening AVIOContext ({})",
                      Error::unpackAVError(ret))));
    }
  }

  return std::unique_ptr<AVFormatContext, FFMpegObjectsDeleter>(ctx);
}
