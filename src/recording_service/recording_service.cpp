#include "recording_service.h"
#include <thread>
#include <chrono>
#include <csignal>
#include <map>
#include <fmt/core.h>
#include "device_context.h"

static bool mustTerminateSignal = false;

int64_t last_video_pts = 0;

void RecordingService::enqueue_video_packet(AVPacket *inputVideoPacket) {
    auto videoPacket = av_packet_clone(inputVideoPacket);

    int64_t pts = av_rescale_q(inputVideoPacket->pts - inputVideoAvs->start_time, inputVideoAvs->time_base,
                               outputVideoAvcc->time_base);

    capturedVideoPacketsQueue.push(std::make_tuple(videoPacket, pts, AVMEDIA_TYPE_VIDEO));
    last_video_pts = pts;
}

int64_t last_audio_pts = 0;

void RecordingService::enqueue_audio_packet(AVPacket *inputAudioPacket) {
    auto audioPacket = av_packet_clone(inputAudioPacket);

    int64_t pts = av_rescale_q(inputAudioPacket->pts - inputAudioAvs->start_time, inputAudioAvs->time_base,
                               outputAudioAvcc->time_base);

    capturedAudioPacketsQueue.push(std::make_tuple(audioPacket, pts, AVMEDIA_TYPE_AUDIO));

    last_audio_pts = pts;
}

int RecordingService::start_capture_loop(bool readFromAux) {
    AVFormatContext *currentAvfc = (!readFromAux) ? inputAvfc : inputAuxAvfc;

    AVPacket inputPacket;
    while (recordingStatus == RECORDING) {
        int ret = av_read_frame(currentAvfc, &inputPacket);
        if (ret == AVERROR(EAGAIN))
            continue;

        if (ret < 0) {
            std::string error = "Capture failed with:";
            error.append(unpackAVError(ret));
            throw std::runtime_error(error);
        }

        AVMediaType packetType = currentAvfc->streams[inputPacket.stream_index]->codecpar->codec_type;

        switch (packetType) {
            case AVMEDIA_TYPE_VIDEO:
                enqueue_video_packet(&inputPacket);
                break;
            case AVMEDIA_TYPE_AUDIO:
                enqueue_audio_packet(&inputPacket);
                break;
            default:
                throw std::runtime_error(
                        build_error_message(__FUNCTION__, {}, fmt::format("unexpected packet type ({})", ret)));
        }
        av_packet_unref(&inputPacket);
    }

    return 0;
}

int64_t last_processed_video_pts = 0;
int64_t last_processed_audio_pts = 0;

int RecordingService::process_captured_packets_queue(bool readFromAux) {

    auto *currentQueue = (!readFromAux) ? &capturedVideoPacketsQueue : &capturedAudioPacketsQueue;

    while (true) {

        if (currentQueue->empty()) {
            if (recordingStatus == RECORDING)
                continue;
            break;
        }

        AVPacket *packet;
        int64_t packetPts;
        AVMediaType packetType;
        std::tie(packet, packetPts, packetType) = currentQueue->front();
        currentQueue->pop();

        int ret;
        switch (packetType) {
            case AVMEDIA_TYPE_VIDEO:
                ret = transcode_video(packet, packetPts);
                last_processed_video_pts = packetPts;
                break;
            case AVMEDIA_TYPE_AUDIO:
                ret = transcode_audio(packet, packetPts);
                last_processed_audio_pts = packetPts;
                break;
            default:
                throw std::runtime_error(
                        build_error_message(__FUNCTION__, {}, fmt::format("unexpected packet type ({})", packetType)));
        }
        if (ret < 0) {
            // Handle
            return -1;
        }

        av_packet_unref(packet);
    }
    return 0;
}

void RecordingService::rec_stats_loop() {
    while (recordingStatus == RECORDING) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::cout << "\r Packet Queue Size: " << capturedVideoPacketsQueue.size()
                  << "Last Captured PTS - video: "
                  << av_rescale_q(last_video_pts, outputVideoAvcc->time_base, {1, 1000}) << " audio: "
                  << av_rescale_q(last_audio_pts, outputAudioAvcc->time_base, {1, 1000})
                  << "Last Processed PTS - video: "
                  << av_rescale_q(last_processed_video_pts, outputVideoAvcc->time_base, {1, 1000}) << " audio: "
                  << av_rescale_q(last_processed_audio_pts, outputAudioAvcc->time_base, {1, 1000});
    }
}

int RecordingService::start_recording() {
    // Write output file header
    if (avformat_write_header(outputAvfc, nullptr) < 0) {
        //Handle
        return -1;
    }

    recordingStatus = RECORDING;

    // Call start_recording_loop in a new thread
    videoCaptureThread = std::thread([this]() {
        start_capture_loop(false);
    });

    if (inputAvfc != inputAuxAvfc) {
        audioCaptureThread = std::thread([this]() {
            start_capture_loop(true);
        });
    }


    capturedVideoPacketsProcessThread = std::thread([this]() {
        process_captured_packets_queue(false);
    });

    capturedAudioPacketsProcessThread = std::thread([this]() {
        process_captured_packets_queue(true);
    });


    recordingStatsThread = std::thread([this]() {
        rec_stats_loop();
    });

    return 0;
}

int RecordingService::pause_recording() {
    // Set pauseTimestamp

    // Set cond var isPaused to true
    recordingStatus = PAUSE;

    return 0;
}

int RecordingService::resume_recording() {
    // Increment pausedTime by resumeTimestamp - pauseTimestamp interval

    // Set cond var isPaused to false
    recordingStatus = RECORDING;
    return 0;
}

int RecordingService::stop_recording() {
    if (recordingStatus == IDLE || recordingStatus == STOP) return 0;
    recordingStatus = STOP;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (videoCaptureThread.joinable())
        videoCaptureThread.join();

    if (audioCaptureThread.joinable())
        audioCaptureThread.join();

    if (capturedVideoPacketsProcessThread.joinable())
        capturedVideoPacketsProcessThread.join();

    if (capturedAudioPacketsProcessThread.joinable())
        capturedAudioPacketsProcessThread.join();

    if (recordingStatsThread.joinable())
        recordingStatsThread.join();


    if (encode_video(0, nullptr)) return -1;
    if (encode_audio_from_buffer(0, true)) return -1;

    if (av_write_trailer(outputAvfc) < 0) {
        std::cout << "write error" << std::endl;
        return -1;
    }

    if (inputAvfc != inputAuxAvfc) {
        avformat_close_input(&inputAuxAvfc);
        avformat_free_context(inputAuxAvfc);
    }
    avformat_close_input(&inputAvfc);
    avformat_free_context(inputAvfc);

    inputAvfc = nullptr;
    inputAuxAvfc = nullptr;
    avformat_close_input(&outputAvfc);
    avformat_free_context(outputAvfc);
    outputAvfc = nullptr;

    avcodec_close(inputVideoAvcc);
    avcodec_free_context(&inputVideoAvcc);
    inputVideoAvcc = nullptr;

    avcodec_close(inputAudioAvcc);
    avcodec_free_context(&inputAudioAvcc);
    inputAudioAvcc = nullptr;

    avcodec_close(outputVideoAvcc);
    avcodec_free_context(&outputVideoAvcc);
    outputVideoAvcc = nullptr;

    avcodec_close(outputAudioAvcc);
    avcodec_free_context(&outputAudioAvcc);
    outputAudioAvcc = nullptr;

    sws_freeContext(videoConverter);
    videoConverter = nullptr;

    swr_free(&audioConverter);
    audioConverter = nullptr;

    av_audio_fifo_free(audioConverterBuffer);
    audioConverterBuffer = nullptr;


    return 0;
}

/// Initializes all the structures needed for the recording process.
/// Accepts deviceAddresses as input in the following format:
///   deviceAddress: "{deviceID}:{url}"
///
/// Examples:
///   - MacOS
///       videoAddress:"avfoundation:1"
///       audioAddress:"avfoundation:1"
///   - Linux
///       videoAddress:"x11grab:...."
///       audioAddress:"pulse:....."
///   - Windows
///       videoAddress:"dshow:...."
///       audioAddress:"dshow:...."
RecordingService::RecordingService(const std::string &videoAddress, const std::string &audioAddress,
                                   const std::string &outputFilename) {
    recordingStatus = IDLE;

    // Initialize the LibAV devices
    avdevice_register_all();

    // Unpack device addresses
    std::string videoDeviceID, videoURL, audioDeviceID, audioURL;
    std::tie(videoDeviceID, videoURL) = unpackDeviceAddress(videoAddress);
    std::tie(audioDeviceID, audioURL) = unpackDeviceAddress(audioAddress);

    // Open A/V devices
    if (videoDeviceID == audioDeviceID) {
        inputDevices.push_back(
                DeviceContext::init_demuxer(videoDeviceID, videoURL, audioURL, get_device_options(videoDeviceID)));
    } else {
        inputDevices.push_back(
                DeviceContext::init_demuxer(videoDeviceID, videoURL, "", get_device_options(videoDeviceID)));
        inputDevices.push_back(
                DeviceContext::init_demuxer(audioDeviceID, "", audioURL, get_device_options(audioDeviceID)));
    }

    // Open output device
    EncoderConfig videoEncoderConfig = {.codecID = AV_CODEC_ID_H264,
            .codecType = AVMEDIA_TYPE_VIDEO,
            .bitRate = OUTPUT_VIDEO_BIT_RATE,
            .height = OUTPUT_HEIGHT,
            .width = OUTPUT_WIDTH,
            .pixelFormat = OUTPUT_VIDEO_PIXEL_FMT,
            .frameRate = OUTPUT_VIDEO_FRAME_RATE};

    EncoderConfig audioEncoderConfig = {.codecID = AV_CODEC_ID_AAC,
            .codecType = AVMEDIA_TYPE_AUDIO,
            .bitRate = OUTPUT_AUDIO_BIT_RATE,
            .channels=
            };
    outputMuxer = DeviceContext::init_muxer(outputFilename, videoEncoderConfig, {});

    // Initialize output streams
    init_output_stream(outputAvfc, &outputVideoAvs);
    init_output_stream(outputAvfc, &outputAudioAvs);

    // Initialize output encoders
    init_video_encoder(inputAvfc, inputVideoAvs, outputVideoAvs, &outputVideoAvc, &outputVideoAvcc);
    init_audio_encoder(inputAudioAvcc, outputAudioAvs, &outputAudioAvc, &outputAudioAvcc);

    // Initialize output converters
    init_video_converter(inputVideoAvcc, outputVideoAvcc, &videoConverter);
    init_audio_converter(inputAudioAvcc, outputAudioAvcc, &audioConverter, &audioConverterBuffer);


    controlThread = std::thread([this]() {
        // Initialize signal to stop recording on sigterm
        std::signal(SIGTERM, [](int) {
            std::cout << "sigterm" << std::endl;
            mustTerminateSignal = true;
        });

        while (!mustTerminateSignal) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (mustTerminateSignal) {
            std::cout << "exiting" << std::endl;
            stop_recording();
            std::cout << "exited" << std::endl;
        }
    });
}

void RecordingService::wait_recording() {
    controlThread.join();
}
