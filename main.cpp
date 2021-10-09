#include "iostream"
#include <vector>
#include <optional>
#include <csignal>
#include <thread>
#include "src/device_service/device_service.h"
#include "src/device_service/input_device.h"
#include "src/recording_service/recording_service.h"

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
};

//
//typedef struct StreamingContext {
//    AVFormatContext *avfc;
//    AVFormatContext *avfcAux;
//    const AVCodec *videoAvc;
//    const AVCodec *audioAvc;
//    AVStream *videoAvs;
//    AVStream *audioAvs;
//    AVCodecContext *videoAvcc;
//    AVCodecContext *audioAvcc;
//    int videoIndex;
//    int audioIndex;
//    char *filename;
//} InputStreamingContext;
//
//typedef struct OutputStreamingContext {
//    AVFormatContext *avfc;
//    const AVCodec *videoAvc;
//    const AVCodec *audioAvc;
//    AVStream *videoAvs;
//    AVStream *audioAvs;
//    AVCodecContext *videoAvcc;
//    AVCodecContext *audioAvcc;
//    int videoIndex;
//    int audioIndex;
//    const char *filename;
//} OutputStreamingContext;
//
//AVFormatContext *openInputDevice(std::string device, std::string url) {
//    AVFormatContext *ctx = avformat_alloc_context();
//    if (!ctx) {
//        // Handle ctx allocation error
//        std::cout << "error during AVFormatContext allocation" << std::endl;
//        return nullptr;
//    }
//
//    int ret;
//
//    auto *inputFormat = av_find_input_format(device.c_str());
//
//    AVDictionary *options = nullptr;
//    ret = av_dict_set(&options, "pixel_format", "nv12",0);
//    if (ret < 0) {
//        std::cout << "error setting input options" << std::endl;
//        return nullptr;
//    }
//    ret = av_dict_set(&options, "framerate", "60",0);
//    if (ret < 0) {
//        std::cout << "error setting input options" << std::endl;
//        return nullptr;
//    }
//    ret = av_dict_set(&options, "capture_cursor", "true",0);
//    if (ret < 0) {
//        std::cout << "error setting input options" << std::endl;
//        return nullptr;
//    }
//
//    ctx->probesize = 100000000;
//    ret = avformat_open_input(&ctx, url.c_str(), inputFormat, &options);
//    if (ret < 0) {
//        // Handle open input error
//        std::cout << "error opening input format" << std::endl;
//        return nullptr;
//    }
//
//    ret = avformat_find_stream_info(ctx, NULL);
//    if (ret < 0) {
//        // Handle find stream info error
//        std::cout << "error finding stream info" << std::endl;
//        return nullptr;
//    }
//    return ctx;
//}
//
//int fill_stream_info(AVStream *avs, const AVCodec **avc, AVCodecContext **avcc) {
//    *avc = avcodec_find_decoder(avs->codecpar->codec_id);
//    if (!*avc) {
//        std::cout << "failed to find the codec" << std::endl;
//        return -1;
//    }
//
//    *avcc = avcodec_alloc_context3(*avc);
//    if (!*avcc) {
//        std::cout << "failed to alloc memory for codec context" << std::endl;
//        return -1;
//    }
//
//    if (avcodec_parameters_to_context(*avcc, avs->codecpar) < 0) {
//        std::cout << "failed to fill codec context" << std::endl;
//        return -1;
//    }
//
//    if (avcodec_open2(*avcc, *avc, NULL) < 0) {
//        std::cout << "failed to open codec" << std::endl;
//        return -1;
//    }
//    return 0;
//}
//
//InputStreamingContext *init_input_stream(std::string videoInDevID, std::string audioInDevID) {
//    auto *inputCtx = (InputStreamingContext *) malloc(sizeof(InputStreamingContext));
//    inputCtx->avfc = openInputDevice("avfoundation", videoInDevID + ":" + audioInDevID);
//    if (!inputCtx->avfc) {
//        // Handle ctx allocation error
//        std::cout << "error opening input device" << std::endl;
//        exit(1);
//    }
//
//    int videoStreamIdx = av_find_best_stream(inputCtx->avfc, AVMEDIA_TYPE_VIDEO, -1, -1, &inputCtx->videoAvc, 0);
//    if (videoStreamIdx < 0) {
//        //Handle negative index
//        std::cout << "error finding video input stream" << std::endl;
//        exit(1);
//    }
//    inputCtx->videoAvs = inputCtx->avfc->streams[videoStreamIdx];
//    inputCtx->videoIndex = videoStreamIdx;
//    fill_stream_info(inputCtx->videoAvs, &inputCtx->videoAvc, &inputCtx->videoAvcc);
//
//    int audioStreamIdx = av_find_best_stream(inputCtx->avfc, AVMEDIA_TYPE_AUDIO, -1, -1, &inputCtx->audioAvc, 0);
//    if (audioStreamIdx < 0) {
//        //Handle negative index
//        std::cout << "error finding audio input stream" << std::endl;
//        exit(1);
//    }
//    inputCtx->audioAvs = inputCtx->avfc->streams[audioStreamIdx];
//    inputCtx->audioIndex = audioStreamIdx;
//    fill_stream_info(inputCtx->audioAvs, &inputCtx->audioAvc, &inputCtx->audioAvcc);
//
//
//    return inputCtx;
//}
//
//OutputStreamingContext *init_output_stream(const std::string &filename) {
//    auto *output = (OutputStreamingContext *) malloc(sizeof(OutputStreamingContext));
//    output->filename = filename.c_str();
//    avformat_alloc_output_context2(&output->avfc, NULL, NULL, output->filename);
//    if (!output->avfc) {
//        //Handle null context
//    }
//
//    return output;
//}
//
//int prepare_audio_encoder(InputStreamingContext *inputCtx, OutputStreamingContext *outputCtx) {
//    outputCtx->audioAvs = avformat_new_stream(outputCtx->avfc, NULL);
//
//    if (!outputCtx->audioAvs) {
//        //Handle err alloc stream
//        std::cout << "error allocating encoder audio AVStream" << std::endl;
//        return -1;
//    }
//    outputCtx->audioAvc = avcodec_find_encoder_by_name("aac");
//
//    if (!outputCtx->audioAvc) {
//        //Handle encoder not found
//        std::cout << "error encoder audio not found" << std::endl;
//        return -1;
//    }
//    outputCtx->audioAvcc = avcodec_alloc_context3(outputCtx->audioAvc);
//    if (!outputCtx->audioAvcc) {
//        //Handle err alloc context
//        std::cout << "error allocating encoder audio AVCodecContext" << std::endl;
//        return -1;
//    }
//
//    int OUTPUT_CHANNELS = 1;
//    int OUTPUT_BIT_RATE = 196000;
//    outputCtx->audioAvcc->channels = OUTPUT_CHANNELS;
//    outputCtx->audioAvcc->channel_layout = av_get_default_channel_layout(OUTPUT_CHANNELS);
//    outputCtx->audioAvcc->sample_rate = inputCtx->audioAvcc->sample_rate;
//    outputCtx->audioAvcc->sample_fmt = outputCtx->audioAvc->sample_fmts[0];
//    outputCtx->audioAvcc->bit_rate = OUTPUT_BIT_RATE;
//    outputCtx->audioAvcc->time_base = (AVRational) {1, inputCtx->audioAvcc->sample_rate};
//
//    outputCtx->audioAvcc->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
//
//    outputCtx->audioAvs->time_base = outputCtx->audioAvcc->time_base;
//
//    if (avcodec_open2(outputCtx->audioAvcc, outputCtx->audioAvc, NULL) < 0) {
//        //Handle
//        std::cout << "error opening enconder audio codec" << std::endl;
//        return -1;
//    }
//    if (avcodec_parameters_from_context(outputCtx->audioAvs->codecpar, outputCtx->audioAvcc)) {
//        //Handle
//        std::cout << "error copying encoder audio stream parameters to codec context" << std::endl;
//        return -1;
//    }
//    return 0;
//}
//
//int prepare_video_encoder(InputStreamingContext *inputCtx, OutputStreamingContext *outputCtx) {
//    AVRational input_framerate = av_guess_frame_rate(inputCtx->avfc, inputCtx->videoAvs, NULL);
//    outputCtx->videoAvs = avformat_new_stream(outputCtx->avfc, NULL);
//
//    if (!outputCtx->videoAvs) {
//        //Handle err alloc stream
//        std::cout << "error allocating encoder video AVStream" << std::endl;
//        return -1;
//    }
//    outputCtx->videoAvc = avcodec_find_encoder(AV_CODEC_ID_H264);
//    if (!outputCtx->videoAvc) {
//        //Handle encoder not found
//        std::cout << "error encoder video not found" << std::endl;
//        return -1;
//    }
//    outputCtx->videoAvcc = avcodec_alloc_context3(outputCtx->videoAvc);
//
//    if (!outputCtx->videoAvcc) {
//        //Handle err alloc context
//        std::cout << "error allocating encoder video AVCodecContext" << std::endl;
//        return -1;
//    }
//    av_opt_set(outputCtx->videoAvcc->priv_data, "preset", "fast", 0);
//    av_opt_set(outputCtx->videoAvcc->priv_data, "x264-params", "keyint=60:min-keyint=60:scenecut=0:force-cfr=1", 0);
//    outputCtx->videoAvcc->height = inputCtx->videoAvcc->height;
//    outputCtx->videoAvcc->width = inputCtx->videoAvcc->width;
//    outputCtx->videoAvcc->sample_aspect_ratio = inputCtx->videoAvcc->sample_aspect_ratio;
//    /*if (outputCtx->videoAvc->pix_fmts)
//        outputCtx->videoAvcc->pix_fmt = outputCtx->videoAvc->pix_fmts[0];
//    else*/
//    outputCtx->videoAvcc->pix_fmt = inputCtx->videoAvcc->pix_fmt;
//    outputCtx->videoAvcc->bit_rate = 2500000;
//
//    outputCtx->videoAvcc->gop_size=10;
//    outputCtx->videoAvcc->max_b_frames=1;
//
//
//    outputCtx->videoAvcc->time_base = av_inv_q(input_framerate);
//    outputCtx->videoAvs->time_base = outputCtx->videoAvcc->time_base;
//    outputCtx->videoAvcc->framerate = input_framerate;
//    outputCtx->videoAvs->avg_frame_rate = input_framerate;
//
//    if (avcodec_open2(outputCtx->videoAvcc, outputCtx->videoAvc, NULL) < 0) {
//        //Handle
//        std::cout << "error opening enconder video codec" << std::endl;
//        return -1;
//    }
//    if (avcodec_parameters_from_context(outputCtx->videoAvs->codecpar, outputCtx->videoAvcc)) {
//        //Handle
//        std::cout << "error copying encoder video stream parameters to codec context" << std::endl;
//        return -1;
//    }
//    return 0;
//
//}
//
//int encode_video(InputStreamingContext *input, OutputStreamingContext *output, AVFrame *videoInputFrame) {
//    if (videoInputFrame) videoInputFrame->pict_type = AV_PICTURE_TYPE_NONE;
//
//
//    AVPacket *output_packet = av_packet_alloc();
//    if (!output_packet) {
//        //Handle
//        std::cout << "error allocating output AVPacket" << std::endl;
//        return -1;
//    }
//
//    int response = avcodec_send_frame(output->videoAvcc, videoInputFrame);
//
//    while (response >= 0) {
//        response = avcodec_receive_packet(output->videoAvcc, output_packet);
//        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
//            break;
//        } else if (response < 0) {
//            //Handle
//            std::cout << "error receiving packet from video encoder" << std::endl;
//            return -1;
//        }
//
//        output_packet->stream_index = input->videoIndex;
//        //output_packet->duration = output->videoAvs->time_base.den / output->videoAvs->time_base.num / 30;
//                                  //input->videoAvs->avg_frame_rate.num * input->videoAvs->avg_frame_rate.den;
//
//        av_packet_rescale_ts(output_packet, input->videoAvs->time_base, output->videoAvs->time_base);
//
//        int outputStartTime = av_rescale_q(input->videoAvs->start_time,input->videoAvs->time_base, output->videoAvs->time_base);
//        output_packet->pts = output_packet->pts - outputStartTime;
//        output_packet->dts = output_packet->dts - outputStartTime;
//
//        //std::cout<<"write frame"<<std::endl;
//        response = av_interleaved_write_frame(output->avfc, output_packet);
//        if (response != 0) {
//            //Handle
//            std::cout << "error writing output frame" << std::endl;
//            return -1;
//        }
//    }
//    av_packet_unref(output_packet);
//    av_packet_free(&output_packet);
//    return 0;
//}
//
//int transcode_video(InputStreamingContext *input, OutputStreamingContext *output, AVPacket *videoInputPacket,
//                    AVFrame *videoInputFrame) {
//    int response = avcodec_send_packet(input->videoAvcc, videoInputPacket);
//    if (response < 0) {
//        std::cout << "error sending packet from video decoder" << std::endl;
//        return -1;
//    }
//
//    while (response >= 0) {
//        response = avcodec_receive_frame(input->videoAvcc, videoInputFrame);
//        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
//            break;
//        } else if (response < 0) {
//            std::cout << "error receiving packet from video decoder" << std::endl;
//            return -1;
//        }
//
//        if (response >= 0) {
//            if (encode_video(input, output, videoInputFrame)) {
//                std::cout << "error encoding frame" << std::endl;
//                return -1;
//            }
//        }
//        av_frame_unref(videoInputFrame);
//    }
//    av_packet_unref(videoInputPacket);
//    return 0;
//}
//
//int encode_audio(InputStreamingContext *input, OutputStreamingContext *output, AVFrame *audioInputFrame) {
//    AVPacket *output_packet = av_packet_alloc();
//    if (!output_packet) {
//        std::cout << "could not allocate memory for output packet" << std::endl;
//        return -1;
//    }
//
//    int response = avcodec_send_frame(output->audioAvcc, audioInputFrame);
//
//    while (response >= 0) {
//        response = avcodec_receive_packet(output->audioAvcc, output_packet);
//        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
//            break;
//        } else if (response < 0) {
//            std::cout << "Error while receiving packet from encoder: " << std::endl;
//            return -1;
//        }
//
//        output_packet->stream_index = input->audioIndex;
//
//        av_packet_rescale_ts(output_packet, input->audioAvs->time_base, output->audioAvs->time_base);
//
//        int outputStartTime = av_rescale_q(input->audioAvs->start_time,input->audioAvs->time_base, output->audioAvs->time_base);
//        output_packet->pts = output_packet->pts - outputStartTime;
//        output_packet->dts = output_packet->dts - outputStartTime;
//
//        response = av_interleaved_write_frame(output->avfc, output_packet);
//        if (response != 0) {
//            std::cout << "Error " << response << " while receiving packet from decoder: " << std::endl;
//            return -1;
//        }
//    }
//    av_packet_unref(output_packet);
//    av_packet_free(&output_packet);
//
//    return 0;
//}
//
//int transcode_audio(InputStreamingContext *input, OutputStreamingContext *output, AVPacket *audioInputPacket,
//                    AVFrame *audioInputFrame) {
//    int response = avcodec_send_packet(input->audioAvcc, audioInputPacket);
//    if (response < 0) {
//        std::cout << "Error while sending packet to decoder: " << std::endl;
//        return response;
//    }
//
//    while (response >= 0) {
//        response = avcodec_receive_frame(input->audioAvcc, audioInputFrame);
//        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
//            break;
//        } else if (response < 0) {
//            std::cout << "Error while receiving frame from input: " << std::endl;
//            return response;
//        }
//
//        if (response >= 0) {
//            if (encode_audio(input, output, audioInputFrame)) return -1;
//        }
//        av_frame_unref(audioInputFrame);
//    }
//
//    av_packet_unref(audioInputPacket);
//    return 0;
//}
//
////int stop_recording(std::tuple<StreamingContext *, OutputStreamingContext *> ctxs) {
//
////}
//bool run = true;
//
//
//int start_recording(std::tuple<InputStreamingContext *, OutputStreamingContext *> ctxs) {
//    InputStreamingContext *input;
//    OutputStreamingContext *output;
//    std::tie(input, output) = ctxs;
//
//    if (avformat_write_header(output->avfc, NULL) < 0) {
//        //Handle
//        return -1;
//    }
//
//    AVFrame *videoInputFrame = av_frame_alloc();
//    if (!videoInputFrame) {
//        //Handle
//        return -1;
//    }
//
//    AVPacket *inputPacket = av_packet_alloc();
//    if (!inputPacket) {
//        //Handle
//        return -1;
//    }
//
//    AVFrame* audioInputFrame = av_frame_alloc();
//    if (!audioInputFrame) {
//        //Handle
//        return -1;
//    }
//
//    AVPacket* audioInputPacket = av_packet_alloc();
//    if (!audioInputPacket) {
//        //Handle
//        return -1;
//    }
//
//
//
//    int res = 0;
//    while (res >= 0) {
//
//        res = av_read_frame(input->avfc, inputPacket);
//
//        if (res == AVERROR(EAGAIN)){
//            //std::cout<<av_err2str(res)<<std::endl;
//            res = 0;
//            continue;
//        }
//
//        std::signal(SIGTERM, [](int) {
//            run = false;
//        });
//
//        if (!run) {
//            break;
//        }
//
//        if (input->avfc->streams[inputPacket->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//            int ret = transcode_video(input, output, inputPacket, videoInputFrame);
//            if (ret < 0) {
//                // Handle
//                return -1;
//            }
//        } else if (input->avfc->streams[inputPacket->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)  {
//            int ret = transcode_audio(input, output, inputPacket, audioInputFrame);
//        if (ret < 0) {
//            // Handle
//            return -1;
//        }
//        }
//
//
//    }
//
//    if (encode_video(input, output, NULL)) return -1;
//    if (encode_audio(input, output, NULL)) return -1;
//
//    av_write_trailer(output->avfc);
//
//    if (videoInputFrame != NULL) {
//        av_frame_free(&videoInputFrame);
//        videoInputFrame = NULL;
//    }
//
//    if (inputPacket != NULL) {
//        av_packet_free(&inputPacket);
//        inputPacket = NULL;
//    }
//
//    if (audioInputFrame != NULL) {
//        av_frame_free(&audioInputFrame);
//        audioInputFrame = NULL;
//    }
//
//    if (audioInputPacket != NULL) {
//        av_packet_free(&audioInputPacket);
//        audioInputPacket = NULL;
//    }
//
//    avformat_close_input(&input->avfc);
//    //avformat_close_input(&input->avfcAux);
//
//    avformat_free_context(input->avfc);
//    input->avfc = NULL;
//    //avformat_free_context(input->avfcAux);
//    //input->avfcAux = NULL;
//    avformat_free_context(output->avfc);
//    output->avfc = NULL;
//
//    avcodec_free_context(&input->videoAvcc);
//    input->videoAvcc = NULL;
//    avcodec_free_context(&input->audioAvcc);
//    input->audioAvcc = NULL;
//
//    free(input);
//    input = NULL;
//    free(output);
//    output = NULL;
//
//    return 0;
//}
//
//std::optional<std::tuple<InputStreamingContext *, OutputStreamingContext *>>
//initRecording(std::string videoInDevID, std::string audioInDevID) {
//    avdevice_register_all();
//    InputStreamingContext *input = init_input_stream(videoInDevID, audioInDevID);
//    OutputStreamingContext *output = init_output_stream("out.mp4");
//    prepare_video_encoder(input, output);
//    prepare_audio_encoder(input, output);
//
//    if (output->avfc->oformat->flags & AVFMT_GLOBALHEADER)
//        output->avfc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
//
//    if (!(output->avfc->oformat->flags & AVFMT_NOFILE)) {
//        if (avio_open(&output->avfc->pb, output->filename, AVIO_FLAG_WRITE) < 0) {
//            //Handle
//            return std::nullopt;
//        }
//    }
//
//
//    return std::make_tuple(input, output);
//
//
//    /*avformat_close_input(&ctx_video);
//    avformat_close_input(&ctx_audio);
//    avformat_free_context(ctx_audio);
//    avformat_free_context(ctx_video);
//    free(input);
//    free(output);*/
//
//}

RecordingService *rs;
#include <chrono>
int main() {
    rs = new RecordingService("avfoundation:1", "avfoundation:1", "output.mp4");
    rs->start_recording();

        std::this_thread::sleep_for(std::chrono::seconds(5));
        rs->stop_recording();


    // List all the available input devices
    std::vector<InputDeviceVideo> videoDevices = DeviceService::get_input_video_devices();
    std::vector<InputDeviceAudio> audioDevices = DeviceService::get_input_audio_devices();

    std::cout << "Video devices:" << std::endl;
    for (InputDeviceVideo inDev : videoDevices) {
        inDev.toString();
    }
    std::cout << "Audio devices:" << std::endl;
    for (InputDeviceAudio inDev : audioDevices) {
        inDev.toString();
    }

    // Select the input devices to capture
    auto videoInDevID = "1";
    auto audioInDevID = "0";

    // Select recording area
    //

//    // Select output file
//    auto outputFile = "out.mkv";
//
//    // Start recording
//    std::optional<std::tuple<InputStreamingContext *, OutputStreamingContext *>> ctxs = initRecording(videoInDevID,
//                                                                                                      audioInDevID);
//    if (ctxs == std::nullopt) {
//        return -1;
//    }
//    start_recording(ctxs.value());
    //  - Allocate input and output contexts
    //  - Open Input Formats (audio video)

    // * startRecording() - main loop - On a different thread
    //  - Read single frame from input streams
    //  - Decode frame
    //  - Add filters (crop)
    //  - Encode frame
    //  - Write interleaved

    // Pause recording

    // Stop recording


    return 0;
}