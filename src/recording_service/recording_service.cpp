#include "recording_service.h"
#include <thread>
#include <csignal>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavutil/channel_layout.h>
}

static bool mustTerminateSignal = false;
static bool mustTerminateStop = false;

AVFormatContext *RecordingService::open_input_device(const std::string &deviceID, const std::string &videoID,
                                                     const std::string &audioID) {
    std::string url = videoID + ":" + audioID;

    AVFormatContext *ctx = avformat_alloc_context();
    if (!ctx) {
        // Handle ctx allocation error
        std::cout << "error during AVFormatContext allocation" << std::endl;
        return nullptr;
    }

    int ret;

    auto *inputFormat = av_find_input_format(deviceID.c_str());

    AVDictionary *options = nullptr;
    ret = av_dict_set(&options, "pixel_format", "nv12", 0);
    if (ret < 0) {
        std::cout << "error setting input options" << std::endl;
        return nullptr;
    }
    ret = av_dict_set(&options, "framerate", "30", 0);
    if (ret < 0) {
        std::cout << "error setting input options" << std::endl;
        return nullptr;
    }
    ret = av_dict_set(&options, "capture_cursor", "true", 0);
    if (ret < 0) {
        std::cout << "error setting input options" << std::endl;
        return nullptr;
    }

    ret = avformat_open_input(&ctx, url.c_str(), inputFormat, &options);
    if (ret < 0) {
        // Handle open input error
        std::cout << "error opening input format" << std::endl;
        return nullptr;
    }

    ctx->probesize = 100000000;
    ret = avformat_find_stream_info(ctx, nullptr);
    if (ret < 0) {
        // Handle find stream info error
        std::cout << "error finding stream info" << std::endl;
        return nullptr;
    }
    return ctx;
}

int RecordingService::open_input_stream_decoder(AVStream *inputStream, const AVCodec **inputCodec,
                                                AVCodecContext **inputCodecCtx) {
    *inputCodec = avcodec_find_decoder(inputStream->codecpar->codec_id);
    if (!*inputCodec) {
        std::cout << "failed to find the codec" << std::endl;
        return -1;
    }

    *inputCodecCtx = avcodec_alloc_context3(*inputCodec);
    if (!*inputCodecCtx) {
        std::cout << "failed to alloc memory for codec context" << std::endl;
        return -1;
    }

    if (avcodec_parameters_to_context(*inputCodecCtx, inputStream->codecpar) < 0) {
        std::cout << "failed to fill codec context" << std::endl;
        return -1;
    }

    if (avcodec_open2(*inputCodecCtx, *inputCodec, nullptr) < 0) {
        std::cout << "failed to open codec" << std::endl;
        return -1;
    }
    return 0;
}

AVFormatContext *RecordingService::open_output_file(const std::string &filename) {
    AVFormatContext *avfc;
    avformat_alloc_output_context2(&avfc, nullptr, nullptr, filename.c_str());
    if (!avfc) {
        //Handle null context
    }

    if (avfc->oformat->flags & AVFMT_GLOBALHEADER)
        avfc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (!(avfc->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&avfc->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0) {
            //Handle
            return nullptr;
        }
    }

    return avfc;
}


int RecordingService::prepare_video_encoder() {
    AVRational input_framerate = av_guess_frame_rate(inputAvfc, inputVideoAvs, nullptr);

    outputVideoAvs = avformat_new_stream(outputAvfc, nullptr);
    if (!outputVideoAvs) {
        //Handle err alloc stream
        std::cout << "error allocating encoder video AVStream" << std::endl;
        return -1;
    }

    outputVideoAvc = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!outputVideoAvc) {
        //Handle encoder not found
        std::cout << "error encoder video not found" << std::endl;
        return -1;
    }
    outputVideoAvcc = avcodec_alloc_context3(outputVideoAvc);

    if (!outputVideoAvcc) {
        //Handle err alloc context
        std::cout << "error allocating encoder video AVCodecContext" << std::endl;
        return -1;
    }
    av_opt_set(outputVideoAvcc->priv_data, "preset", "fast", 0);
    av_opt_set(outputVideoAvcc->priv_data, "x264-params", "keyint=60:min-keyint=60:scenecut=0:force-cfr=1",
               0);
    outputVideoAvcc->height = inputVideoAvcc->height;
    outputVideoAvcc->width = inputVideoAvcc->width;
    outputVideoAvcc->sample_aspect_ratio = inputVideoAvcc->sample_aspect_ratio;
    /*if (outputCtx->videoAvc->pix_fmts)
        outputVideoAvcc->pix_fmt = outputCtx->videoAvc->pix_fmts[0];
    else*/
    outputVideoAvcc->pix_fmt = inputVideoAvcc->pix_fmt;
    outputVideoAvcc->bit_rate = 2500000;

    outputVideoAvcc->gop_size = 10;
    outputVideoAvcc->max_b_frames = 1;


    outputVideoAvcc->time_base = av_inv_q(input_framerate);
    outputVideoAvs->time_base = outputVideoAvcc->time_base;
    outputVideoAvcc->framerate = input_framerate;
    outputVideoAvs->avg_frame_rate = input_framerate;

    if (avcodec_open2(outputVideoAvcc, outputVideoAvc, nullptr) < 0) {
        //Handle
        std::cout << "error opening enconder video codec" << std::endl;
        return -1;
    }
    if (avcodec_parameters_from_context(outputVideoAvs->codecpar, outputVideoAvcc)) {
        //Handle
        std::cout << "error copying encoder video stream parameters to codec context" << std::endl;
        return -1;
    }
    return 0;

}


int RecordingService::prepare_audio_encoder() {
    outputAudioAvs = avformat_new_stream(outputAvfc, nullptr);

    if (!outputAudioAvs) {
        //Handle err alloc stream
        std::cout << "error allocating encoder audio AVStream" << std::endl;
        return -1;
    }
    outputAudioAvc = avcodec_find_encoder_by_name("aac");

    if (!outputAudioAvc) {
        //Handle encoder not found
        std::cout << "error encoder audio not found" << std::endl;
        return -1;
    }

    outputAudioAvcc = avcodec_alloc_context3(outputAudioAvc);
    if (!outputAudioAvcc) {
        //Handle err alloc context
        std::cout << "error allocating encoder audio AVCodecContext" << std::endl;
        return -1;
    }

    int OUTPUT_CHANNELS = 1;
    int OUTPUT_BIT_RATE = 196000;
    outputAudioAvcc->channels = OUTPUT_CHANNELS;
    outputAudioAvcc->channel_layout = av_get_default_channel_layout(OUTPUT_CHANNELS);
    outputAudioAvcc->sample_rate = inputAudioAvcc->sample_rate;
    outputAudioAvcc->sample_fmt = outputAudioAvc->sample_fmts[0];
    outputAudioAvcc->bit_rate = OUTPUT_BIT_RATE;
    outputAudioAvcc->time_base = (AVRational) {1, inputAudioAvcc->sample_rate};

    outputAudioAvcc->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    outputAudioAvs->time_base = outputAudioAvcc->time_base;

    if (avcodec_open2(outputAudioAvcc, outputAudioAvc, nullptr) < 0) {
        //Handle
        std::cout << "error opening enconder audio codec" << std::endl;
        return -1;
    }
    if (avcodec_parameters_from_context(outputAudioAvs->codecpar, outputAudioAvcc)) {
        //Handle
        std::cout << "error copying encoder audio stream parameters to codec context" << std::endl;
        return -1;
    }
    return 0;
}

int RecordingService::encode_video(AVFrame *videoInputFrame) {
    if (videoInputFrame) videoInputFrame->pict_type = AV_PICTURE_TYPE_NONE;


    AVPacket *output_packet = av_packet_alloc();
    if (!output_packet) {
        //Handle
        std::cout << "error allocating output AVPacket" << std::endl;
        return -1;
    }

    int response = avcodec_send_frame(outputVideoAvcc, videoInputFrame);

    while (response >= 0) {
        response = avcodec_receive_packet(outputVideoAvcc, output_packet);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            std::cout << "eagain" << std::endl;
            break;
        } else if (response < 0) {
            //Handle
            std::cout << "error receiving packet from video encoder" << std::endl;
            return -1;
        }

        output_packet->stream_index = inputVideoIndex;
        //output_packet->duration = outputCtx->videoAvs->time_base.den / outputCtx->videoAvs->time_base.num / 30;
        //inputCtx->videoAvs->avg_frame_rate.num * inputCtx->videoAvs->avg_frame_rate.den;

        av_packet_rescale_ts(output_packet, inputVideoAvs->time_base, outputVideoAvs->time_base);

        int outputStartTime = av_rescale_q(inputVideoAvs->start_time, inputVideoAvs->time_base,
                                           outputVideoAvs->time_base);
        output_packet->pts = output_packet->pts - outputStartTime;
        output_packet->dts = output_packet->dts - outputStartTime;

        //std::cout<<"write frame"<<std::endl;
        response = av_interleaved_write_frame(outputAvfc, output_packet);
        if (response != 0) {
            //Handle
            std::cout << "error writing output frame" << std::endl;
            return -1;
        }
    }
    av_packet_unref(output_packet);
    av_packet_free(&output_packet);
    return 0;
}


int RecordingService::transcode_video(AVPacket *videoInputPacket,
                    AVFrame *videoInputFrame) {
    int response = avcodec_send_packet(inputVideoAvcc, videoInputPacket);
    if (response < 0) {
        std::cout << "error sending packet from video decoder" << std::endl;
        return -1;
    }

    while (response >= 0) {
        response = avcodec_receive_frame(inputVideoAvcc, videoInputFrame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            break;
        } else if (response < 0) {
            std::cout << "error receiving packet from video decoder" << std::endl;
            return -1;
        }

        if (response >= 0) {
            if (encode_video(videoInputFrame)) {
                std::cout << "error encoding frame" << std::endl;
                return -1;
            }
        }
        av_frame_unref(videoInputFrame);
    }
    av_packet_unref(videoInputPacket);
    return 0;
}

int RecordingService::encode_audio(AVFrame *audioInputFrame) {
    AVPacket *output_packet = av_packet_alloc();
    if (!output_packet) {
        std::cout << "could not allocate memory for output packet" << std::endl;
        return -1;
    }

    int response = avcodec_send_frame(outputAudioAvcc, audioInputFrame);

    while (response >= 0) {
        response = avcodec_receive_packet(outputAudioAvcc, output_packet);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            break;
        } else if (response < 0) {
            std::cout << "Error while receiving packet from encoder: " << std::endl;
            return -1;
        }

        output_packet->stream_index = inputAudioIndex;

        av_packet_rescale_ts(output_packet, inputAudioAvs->time_base, outputAudioAvs->time_base);

        int outputStartTime = av_rescale_q(inputAudioAvs->start_time, inputAudioAvs->time_base,
                                           outputAudioAvs->time_base);
        output_packet->pts = output_packet->pts - outputStartTime;
        output_packet->dts = output_packet->dts - outputStartTime;

        response = av_interleaved_write_frame(outputAvfc, output_packet);
        if (response != 0) {
            std::cout << "Error " << response << " while receiving packet from decoder: " << std::endl;
            return -1;
        }
    }
    av_packet_unref(output_packet);
    av_packet_free(&output_packet);

    return 0;
}

int RecordingService::transcode_audio(AVPacket *audioInputPacket,
                    AVFrame *audioInputFrame) {
    int response = avcodec_send_packet(inputAudioAvcc, audioInputPacket);
    if (response < 0) {
        std::cout << "Error while sending packet to decoder: " << std::endl;
        return response;
    }

    while (response >= 0) {
        response = avcodec_receive_frame(inputAudioAvcc, audioInputFrame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            break;
        } else if (response < 0) {
            std::cout << "Error while receiving frame from input: " << std::endl;
            return response;
        }

        if (response >= 0) {
            if (encode_audio(audioInputFrame)) return -1;
        }
        av_frame_unref(audioInputFrame);
    }

    av_packet_unref(audioInputPacket);
    return 0;
}

int
RecordingService::start_recording_loop() {
    // if inputAuxCtx == nullptr
    //  -> read frame from device (loop not blocked)
    //  -> open video thread
    //      -> transcode frame
    //      -> write interleaved frame
    //  -> open audio thread
    //      -> transcode frame
    //      -> write interleaved frame
    // -> else
    //  -> open video thread
    //      -> read frame from device
    //      -> transcode frame
    //      -> write interleaved frame
    //  -> open audio thread
    //      -> read frame from device
    //      -> transcode frame
    //      -> write interleaved frame

    AVFrame *videoInputFrame = av_frame_alloc();
    if (!videoInputFrame) {
        //Handle
        return -1;
    }

    AVPacket *inputPacket = av_packet_alloc();
    if (!inputPacket) {
        //Handle
        return -1;
    }

    AVFrame *audioInputFrame = av_frame_alloc();
    if (!audioInputFrame) {
        //Handle
        return -1;
    }

    AVPacket *audioInputPacket = av_packet_alloc();
    if (!audioInputPacket) {
        //Handle
        return -1;
    }

    if (inputAuxAvfc == nullptr) {
        int res = 0;
        while (res >= 0) {

            res = av_read_frame(inputAvfc, inputPacket);

            if (res == AVERROR(EAGAIN)) {
                res = 0;
                continue;
            }

            if (inputAvfc->streams[inputPacket->stream_index]->codecpar->codec_type ==
                AVMEDIA_TYPE_VIDEO) {
                int ret = transcode_video(inputPacket, videoInputFrame);
                if (ret < 0) {
                    // Handle
                    return -1;
                }
            } else if (inputAvfc->streams[inputPacket->stream_index]->codecpar->codec_type ==
                       AVMEDIA_TYPE_AUDIO) {
                int ret = transcode_audio(inputPacket, audioInputFrame);
                if (ret < 0) {
                    // Handle
                    return -1;
                }
            }

            if (mustTerminateSignal || mustTerminateStop) {
                break;
            }
        }
    }

    if (encode_video(nullptr)) return -1;
    if (encode_audio(nullptr)) return -1;

    if (videoInputFrame != nullptr) {
        av_frame_free(&videoInputFrame);
        videoInputFrame = nullptr;
    }

    if (inputPacket != nullptr) {
        av_packet_free(&inputPacket);
        inputPacket = nullptr;
    }

    if (audioInputFrame != nullptr) {
        av_frame_free(&audioInputFrame);
        audioInputFrame = nullptr;
    }

    if (audioInputPacket != nullptr) {
        av_packet_free(&audioInputPacket);
        audioInputPacket = nullptr;
    }


    if (av_write_trailer(outputAvfc) < 0){
        std::cout<<"write error"<<std::endl;
        return -1;
    }

    avformat_close_input(&inputAvfc);
    //avformat_close_input(&this->inputCtx->avfcAux);

    avformat_free_context(inputAvfc);
    inputAvfc = nullptr;
    //avformat_free_context(this->inputCtx->avfcAux);
    //this->inputCtx->avfcAux = NULL;
    avformat_free_context(outputAvfc);
    outputAvfc = nullptr;

    avcodec_free_context(&inputVideoAvcc);
    inputVideoAvcc = nullptr;

    avcodec_free_context(&inputAudioAvcc);
    inputAudioAvcc = nullptr;

    if (mustTerminateSignal){
        stop_recording();
    }


    return 0;
}

int RecordingService::start_recording() {
    // Write output file header
    if (avformat_write_header(outputAvfc, nullptr) < 0) {
        //Handle
        return -1;
    }


    // Call start_recording_loop in a new thread

    //std::thread thread([this]() {
        start_recording_loop();
    //});

    //thread.join();

    return 0;
}

int RecordingService::pause_recording() {
    // Set pauseTimestamp

    // Set cond var isPaused to true
    return 0;
}

int RecordingService::resume_recording() {
    // Increment pausedTime by resumeTimestamp - pauseTimestamp interval

    // Set cond var isPaused to false
    return 0;
}

int RecordingService::stop_recording() {
    // Stop recording loop thread (set cond var stopRecording to true)
    // Flush encoders
    // Write output file trailer
    // Free res (?)
    mustTerminateStop = true;



    return 0;
}

/// Initializes all the structures needed for the recording process.
/// Accepts device IDs as input in the following format:
/// - MacOS
///     videoInDevID:"avfoundation:1"
///     audioInDevID:"avfoundation:1"
/// - Linux
///     videoInDevID:"x11grab:...."
///     audioInDevID:"pulse:....."
/// - Windows
///     videoInDevID:"dshow:...."
///     audioInDevID:"dshow:...."
RecordingService::RecordingService(const std::string &videoInDevID, const std::string &audioInDevID,
                                   const std::string &outputFilename) {

    // Split input devices ID
    // If audio and video deviceID is equal
    //  -> call open_input_device(deviceID, videoID, audioID)
    //  -> get input streamID for audio and video
    //  -> call open_input_stream_decoder for audio and video
    //  -> build single InputStreamingContext
    // Else
    //  -> call open_input_device(deviceID, videoID, nullptr) and open_input_device(deviceID, nullptr, audioID)
    //  -> get input streamID for audio and video
    //  -> call open_input_stream_decoder for audio and video
    //  -> build double InputStreamingContext
    avdevice_register_all();

    int delimiterIndex = videoInDevID.find(':');
    std::string videoDeviceID = videoInDevID.substr(0, delimiterIndex);
    std::string videoURL = videoInDevID.substr(delimiterIndex + 1, videoInDevID.length());

    delimiterIndex = audioInDevID.find(':');
    std::string audioDeviceID = audioInDevID.substr(0, delimiterIndex);
    std::string audioURL = audioInDevID.substr(delimiterIndex + 1, audioInDevID.length());

    if (videoDeviceID == audioDeviceID) {
        // Open A/V device
        inputAvfc = open_input_device(videoDeviceID, videoURL, audioURL);

        // Get video stream
        inputVideoIndex = av_find_best_stream(inputAvfc, AVMEDIA_TYPE_VIDEO, -1, -1, &inputVideoAvc, 0);
        if (inputVideoIndex < 0) {
            //Handle negative index
            std::cout << "error finding video input stream" << std::endl;
            exit(1);
        }
        inputVideoAvs = inputAvfc->streams[inputVideoIndex];

        // Open video decoder
        open_input_stream_decoder(inputVideoAvs, &inputVideoAvc, &inputVideoAvcc);

        // Get audio stream
        inputAudioIndex = av_find_best_stream(inputAvfc, AVMEDIA_TYPE_AUDIO, -1, -1, &inputAudioAvc, 0);
        if (inputAudioIndex < 0) {
            //Handle negative index
            std::cout << "error finding audio input stream" << std::endl;
            exit(1);
        }
        inputAudioAvs = inputAvfc->streams[inputAudioIndex];

        // Open audio decoder
        open_input_stream_decoder(inputAudioAvs, &inputAudioAvc, &inputAudioAvcc);
        std::cout << "maybe" << std::endl;

    } else {

    }

    // Call open_output_file
    outputAvfc = open_output_file(outputFilename);

    // Set class attributes
    prepare_video_encoder();
    prepare_audio_encoder();

    this->inputAuxAvfc = nullptr;

    // Initialize signal to stop recording on sigterm
    std::signal(SIGTERM, [](int) {
        std::cout << "echo"<<std::endl;
        mustTerminateSignal = true;
    });
}