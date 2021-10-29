#include <fmt/core.h>
#include "recording_service.h"

/// Initializes the device identified by the passed deviceID.
AVFormatContext *RecordingService::init_input_device(const std::string &deviceID, const std::string &videoURL,
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
    if (!videoURL.empty() && audioURL.empty()){
        url = videoURL;
    } else if(videoURL.empty() && !audioURL.empty()){
        url = audioURL;
    } else{
        url = videoURL + ":" + audioURL;
    }
    ret = avformat_open_input(&ctx, url.c_str(), inputFormat, &options);
    if (ret < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, methodParams,
                                                     fmt::format("error opening input format ({})", unpackAVError(ret))));
    }

    // Find device's streams info
    ctx->probesize = 100000000; // size of the buffer containing the frames used to get streams info
    ret = avformat_find_stream_info(ctx, nullptr);
    if (ret < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, methodParams,
                                                     fmt::format("error finding streams info ({})", unpackAVError(ret))));
    }

    return ctx;
}

/// Finds the main video stream in the input device context and its decoder.
void RecordingService::init_video_input_stream(AVFormatContext *inputAvfc, AVStream **inputVideoAvs,
                                            AVCodec **inputVideoAvc) {
    int inputVideoIndex = av_find_best_stream(inputAvfc, AVMEDIA_TYPE_VIDEO, -1, -1, inputVideoAvc, 0);
    if (inputVideoIndex < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error finding main video stream"));
    }

    *inputVideoAvs = inputAvfc->streams[inputVideoIndex];
}

/// Finds the main audio stream in the input device context and its decoder.
void RecordingService::init_audio_input_stream(AVFormatContext *inputAvfc, AVStream **inputAudioAvs,
                                            AVCodec **inputAudioAvc) {
    int inputAudioIndex = av_find_best_stream(inputAvfc, AVMEDIA_TYPE_AUDIO, -1, -1, inputAudioAvc, 0);
    if (inputAudioIndex < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error finding main audio stream"));
    }

    *inputAudioAvs = inputAvfc->streams[inputAudioIndex];
}

/// Initializes the decoder for the passed input stream.
/// If no codec is provided, it will be searched using the stream's codecID.
void RecordingService::init_input_stream_decoder(AVStream *inputAvs, AVCodec **inputAvc,
                                                 AVCodecContext **inputAvcc) {
    if (*inputAvc == nullptr) {
        *inputAvc = avcodec_find_decoder(inputAvs->codecpar->codec_id);
        if (!*inputAvc) {
            throw std::runtime_error(build_error_message(__FUNCTION__, {}, "no decoder found for the input stream"));
        }
    }

    *inputAvcc = avcodec_alloc_context3(*inputAvc);
    if (!*inputAvcc) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error allocating context for the decoder"));
    }

    int ret;
    ret = avcodec_parameters_to_context(*inputAvcc, inputAvs->codecpar);
    if (ret < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {},
                                                     fmt::format("error copying decoder parameters to the context ({})",
                                                                 unpackAVError(ret))));
    }

    ret = avcodec_open2(*inputAvcc, *inputAvc, nullptr);
    if (ret < 0) {
        throw std::runtime_error(
                build_error_message(__FUNCTION__, {}, fmt::format("error opening decoder ({})", unpackAVError(ret))));
    }
}

/// Allocates the output context and open the output file.
AVFormatContext *RecordingService::init_output_context_and_file(const std::string &outputFileName) {
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

/// Allocates an output stream.
void RecordingService::init_output_stream(AVFormatContext *outputAvfc, AVStream **outputAvs) {
    *outputAvs = avformat_new_stream(outputAvfc, nullptr);
    if (!*outputAvs) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error allocating output AVStream"));
    }

    (*outputAvs)->time_base = {1, 1000};
}

/// Initializes the encoder for the video output stream.
void RecordingService::init_video_encoder(AVFormatContext *inputAvfc, AVStream *inputVideoAvs, AVStream *outputVideoAvs,
                                          const AVCodec **outputVideoAvc, AVCodecContext **outputVideoAvcc) {
    // Find encoder for output stream
    *outputVideoAvc = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!*outputVideoAvc) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error finding encoder 'AV_CODEC_ID_H264'"));
    }

    // Allocate context for the encoder
    *outputVideoAvcc = avcodec_alloc_context3(*outputVideoAvc);
    if (!*outputVideoAvcc) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error allocating context for the encoder"));
    }

    // Set the encoder options
    std::map<std::string, std::string> encoderOptions = {{"preset",      "ultrafast"},
                                                         {"x264-params", "keyint=60:min-keyint=60:scenecut=0:force-cfr=1"}};

    int ret;
    for (const auto &option:encoderOptions) {
        ret = av_opt_set((*outputVideoAvcc)->priv_data, option.first.c_str(), option.second.c_str(), 0);
        if (ret < 0) {
            throw std::runtime_error(build_error_message(__FUNCTION__, {}, fmt::format(
                    "error setting '{}' encoder option to value '{}' ({})",
                    option.first, option.second, unpackAVError(ret))));
        }
    }

    // Fill encoder params
    (*outputVideoAvcc)->height = OUTPUT_HEIGHT;
    (*outputVideoAvcc)->width = OUTPUT_WIDTH;
    (*outputVideoAvcc)->sample_aspect_ratio = inputVideoAvs->codecpar->sample_aspect_ratio;
    (*outputVideoAvcc)->pix_fmt = OUTPUT_VIDEO_PIXEL_FMT;
    (*outputVideoAvcc)->bit_rate = OUTPUT_VIDEO_BIT_RATE;

    AVRational input_framerate = av_guess_frame_rate(inputAvfc, inputVideoAvs, nullptr);
    (*outputVideoAvcc)->time_base = av_inv_q(input_framerate);

    // TODO: is this useful?
    // set codec to automatically determine how many threads suits best for the decoding job
    (*outputVideoAvcc)->thread_count = 0;

    if ((*outputVideoAvc)->capabilities | AV_CODEC_CAP_FRAME_THREADS)
        (*outputVideoAvcc)->thread_type = FF_THREAD_FRAME;
    else if ((*outputVideoAvc)->capabilities | AV_CODEC_CAP_SLICE_THREADS)
        (*outputVideoAvcc)->thread_type = FF_THREAD_SLICE;
    else
        (*outputVideoAvcc)->thread_count = 1; //don't use multithreading

    // Open encoder
    ret = avcodec_open2(*outputVideoAvcc, *outputVideoAvc, nullptr);
    if (ret < 0) {
        throw std::runtime_error(
                build_error_message(__FUNCTION__, {}, fmt::format("error opening encoder ({})", unpackAVError(ret))));
    }

    // Copy encoder parameter to stream
    ret = avcodec_parameters_from_context(outputVideoAvs->codecpar, *outputVideoAvcc);
    if (ret < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {},
                                                     fmt::format("error copying encoder parameters to the stream ({})",
                                                                 unpackAVError(ret))));
    }
}

/// Initializes the encoder for the audio output stream.
void RecordingService::init_audio_encoder(AVCodecContext *inputAudioAvcc, AVStream *outputAudioAvs,
                                          const AVCodec **outputAudioAvc, AVCodecContext **outputAudioAvcc) {
    // Find encoder for output stream
    *outputAudioAvc = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!*outputAudioAvc) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error finding encoder 'AV_CODEC_ID_AAC'"));
    }

    // Allocate context for the encoder
    *outputAudioAvcc = avcodec_alloc_context3(*outputAudioAvc);
    if (!*outputAudioAvcc) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {}, "error allocating context for the encoder"));
    }

    // Fill encoder params
    (*outputAudioAvcc)->channels = inputAudioAvcc->channels;
    (*outputAudioAvcc)->channel_layout = av_get_default_channel_layout(inputAudioAvcc->channels);
    (*outputAudioAvcc)->sample_rate = inputAudioAvcc->sample_rate;
    (*outputAudioAvcc)->sample_fmt = OUTPUT_AUDIO_SAMPLE_FMT;
    (*outputAudioAvcc)->bit_rate = OUTPUT_AUDIO_BIT_RATE;
    (*outputAudioAvcc)->time_base = {1, inputAudioAvcc->sample_rate};
    (*outputAudioAvcc)->strict_std_compliance = FF_COMPLIANCE_NORMAL;

    // Open encoder
    int ret = avcodec_open2(*outputAudioAvcc, *outputAudioAvc, nullptr);
    if (ret < 0) {
        throw std::runtime_error(
                build_error_message(__FUNCTION__, {}, fmt::format("error opening encoder ({})", unpackAVError(ret))));
    }

    // Copy encoder parameter to stream
    ret = avcodec_parameters_from_context(outputAudioAvs->codecpar, *outputAudioAvcc);
    if (ret < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {},
                                                     fmt::format("error copying encoder parameters to the stream ({})",
                                                                 unpackAVError(ret))));
    }
}

/// Initializes the video scaler.
void RecordingService::init_video_converter(AVCodecContext *inputVideoAvcc, AVCodecContext *outputVideoAvcc,
                                            SwsContext **videoConverter) {
    *videoConverter = sws_getContext(inputVideoAvcc->width, inputVideoAvcc->height, inputVideoAvcc->pix_fmt,
                                     outputVideoAvcc->width, outputVideoAvcc->height, OUTPUT_VIDEO_PIXEL_FMT,
                                     SWS_BICUBIC,
                                     nullptr, nullptr, nullptr);
    if (!*videoConverter) {
        throw std::runtime_error(
                build_error_message(__FUNCTION__, {}, "error initializing video converter"));
    }
}

/// Initializes the audio converter.
void RecordingService::init_audio_converter(AVCodecContext *inputAudioAvcc, AVCodecContext *outputAudioAvcc,
                                            SwrContext **audioConverter, AVAudioFifo **audioConverterBuffer) {
    // Allocate audio converter context
    *audioConverter = swr_alloc_set_opts(nullptr,
                                         outputAudioAvcc->channel_layout,
                                         outputAudioAvcc->sample_fmt,  // aac encoder only receive this format
                                         outputAudioAvcc->sample_rate,
                                         av_get_default_channel_layout(inputAudioAvcc->channels),
                                         inputAudioAvcc->sample_fmt,
                                         inputAudioAvcc->sample_rate,
                                         0, nullptr);
    if (!*audioConverter) {
        throw std::runtime_error(
                build_error_message(__FUNCTION__, {}, "error allocating audio converter context"));
    }

    // Initialize the audio converter
    int ret = swr_init(*audioConverter);
    if (ret < 0) {
        throw std::runtime_error(build_error_message(__FUNCTION__, {},
                                                     fmt::format("error initializing audio converter ({})",
                                                                 unpackAVError(ret))));
    }

    // Allocate a 2 seconds FIFO buffer for converting
    *audioConverterBuffer = av_audio_fifo_alloc(OUTPUT_AUDIO_SAMPLE_FMT, inputAudioAvcc->channels,
                                                outputAudioAvcc->sample_rate * 2);
    if (!*audioConverterBuffer) {
        throw std::runtime_error(
                build_error_message(__FUNCTION__, {}, "error allocating audio converter buffer"));
    }
}