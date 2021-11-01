#include "device_context.h"
#include "error.h"
#include <fmt/core.h>

DeviceContext DeviceContext::init_demuxer(const std::string &deviceID, const std::string &videoURL,
                                          const std::string &audioURL,
                                          const std::map<std::string, std::string> &optionsMap) {
    DeviceContext deviceContext;
    deviceContext.avfc = init_input_device(deviceID, videoURL, audioURL, optionsMap);

    if (!videoURL.empty()) {
        int videoStreamID = deviceContext.find_main_stream(AVMEDIA_TYPE_VIDEO);
        deviceContext.streams.push_back(StreamContext::from_input_stream(deviceContext.avfc->streams[videoStreamID]));
    }

    if (!audioURL.empty()) {
        int audioStreamID = deviceContext.find_main_stream(AVMEDIA_TYPE_AUDIO);
        deviceContext.streams.push_back(StreamContext::from_input_stream(deviceContext.avfc->streams[audioStreamID]));
    }
    return deviceContext;
}

DeviceContext DeviceContext::init_muxer(const std::string &outputFileName,
                                        const EncoderConfig &videoEncoderConfig,
                                        const EncoderConfig &audioEncoderConfig) {
    DeviceContext deviceContext;

    deviceContext.avfc = init_output_context(outputFileName);

    // Add new output video stream
    deviceContext.streams.push_back(StreamContext::new_output_stream(deviceContext.avfc, videoEncoderConfig));
    // Add new output audio stream
    deviceContext.streams.push_back(StreamContext::new_output_stream(deviceContext.avfc, audioEncoderConfig));

    return deviceContext;
}


/// Initializes the device identified by the passed deviceID.
AVFormatContext *DeviceContext::init_input_device(const std::string &deviceID, const std::string &videoURL,
                                                  const std::string &audioURL,
                                                  const std::map<std::string, std::string> &optionsMap) {
    int ret;
    // Build method params for error handling purposes
    std::map<std::string, std::string> methodParams = {{"deviceID", deviceID},
                                                       {"videoURL", videoURL},
                                                       {"audioURL", audioURL}};

    // Allocate a context for the device
    AVFormatContext *ctx = avformat_alloc_context();
    if (!ctx) {
        throw std::runtime_error(build_error_message(__FUNCTION__, methodParams,
                                                     "error during AVFormatContext allocation"));
    }

    // Find the input format associated to the deviceID
    AVInputFormat *inputFormat = av_find_input_format(deviceID.c_str());
    if (!inputFormat) {
        throw std::runtime_error(build_error_message(__FUNCTION__, methodParams,
                                                     "no AVInputFormat found for deviceID"));
    }

    // Build the options for the device
    AVDictionary *options = nullptr;
    for (const auto &option:optionsMap) {
        ret = av_dict_set(&options, option.first.c_str(), option.second.c_str(), 0);
        if (ret < 0) {
            throw std::runtime_error(build_error_message(__FUNCTION__, methodParams,
                                                         fmt::format("error setting '{}' option to value '{}' ({})",
                                                                     option.first, option.second, unpackAVError(ret))));
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
        throw std::runtime_error(build_error_message(__FUNCTION__, methodParams,
                                                     fmt::format("error opening input format ({})",
                                                                 unpackAVError(ret))));
    }

    // Find device's streams info
    ctx->probesize = 100000000; // size of the buffer containing the frames used to get streams info
    ret = avformat_find_stream_info(ctx, nullptr);
    if (ret < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, methodParams,
                                                     fmt::format("error finding streams info ({})",
                                                                 unpackAVError(ret))));
    }

    return ctx;
}

/// Finds the main video stream in the current device context.
int DeviceContext::find_main_stream(AVMediaType streamType) {
    if (avfc == nullptr) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "device not initialized"));
    }

    int streamIndex = av_find_best_stream(avfc, streamType, -1, -1, nullptr, 0);
    if (streamIndex < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {},
                                                     fmt::format("error finding main stream for type {}", streamType)));
    }

    return streamIndex;
}


/// Allocates the output context and open the output file.
AVFormatContext *DeviceContext::init_output_context(const std::string &outputFileName) {
    // Build method params for error handling purposes
    std::map<std::string, std::string> methodParams = {{"outputfileName", outputFileName}};

    AVFormatContext *ctx;
    int ret = avformat_alloc_output_context2(&ctx, nullptr, nullptr, outputFileName.c_str());
    if (ret < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, methodParams,
                                                     fmt::format("error during output AVFormatContext allocation ({})",
                                                                 unpackAVError(ret))));
    }

    if (ctx->oformat->flags & AVFMT_GLOBALHEADER)
        ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (!(ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ctx->pb, outputFileName.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            throw std::runtime_error(build_error_message(__FUNCTION__, methodParams, fmt::format(
                    "error opening AVIOContext ({})", unpackAVError(ret))));
        }
    }

    return ctx;
}

