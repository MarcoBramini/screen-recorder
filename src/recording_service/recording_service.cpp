#include "recording_service.h"
#include <thread>
#include <chrono>
#include <csignal>
#include <map>
#include <fmt/core.h>

static bool mustTerminateSignal = false;
static bool mustTerminateStop = false;

bool readAux = true;

/// Read a new packet from the input device(s) in Round Robin fashion.
/// If the device is the same for audio and video, it reads from the same context for both audio and video.
/// On success, it returns 0 (video) or 1 (audio) depending on the packet's nature.
/// Returns the AVERROR, returned by the av_read_frame function, on failure.
int read_next_frame(AVFormatContext *inputAvfc, AVFormatContext *inputAuxAvfc, AVPacket *inputPacket) {
    readAux = !readAux;
    // Select the device context to use
    AVFormatContext *currentCtx = (!readAux) ? inputAvfc : inputAuxAvfc;

    // Read next frame and fill the return packet
    int ret = av_read_frame(currentCtx, inputPacket);
    if (ret < 0) {
        return ret;
    }

    return currentCtx->streams[inputPacket->stream_index]->codecpar->codec_type;
}

int64_t last_video_pts = 0;

void RecordingService::enqueue_video_packet(AVPacket *inputVideoPacket) {
    auto videoPacket = av_packet_clone(inputVideoPacket);
    capturedPacketsQueue.push(std::make_tuple(videoPacket, last_video_pts, AVMEDIA_TYPE_VIDEO));
    last_video_pts += (int) (1000 / 30);
}

int64_t last_audio_pts = 0;

void RecordingService::enqueue_audio_packet(AVPacket *inputAudioPacket) {
    int64_t frameDuration = av_get_audio_frame_duration(outputAudioAvcc, outputAudioAvcc->frame_size);

    auto audioPacket = av_packet_clone(inputAudioPacket);
    capturedPacketsQueue.push(std::make_tuple(audioPacket, last_audio_pts, AVMEDIA_TYPE_AUDIO));
    last_audio_pts += frameDuration / 2;
}

int RecordingService::start_capture_loop() {
    AVPacket *inputPacket = av_packet_alloc();
    if (!inputPacket) {
        //Handle
        return -1;
    }


    while (true) {
        int res = read_next_frame(inputAvfc, inputAuxAvfc, inputPacket);
        if (res == AVERROR(EAGAIN))
            continue;

        if (res < 0) {
            std::string error = "Capture failed with:";
            error.append(unpackAVError(res));
            throw std::runtime_error(error);
        }

        switch (res) {
            case AVMEDIA_TYPE_VIDEO:
                enqueue_video_packet(inputPacket);
                break;
            case AVMEDIA_TYPE_AUDIO:
                enqueue_audio_packet(inputPacket);
                break;
            default:
                throw std::runtime_error(
                        build_error_message(__FUNCTION__, {}, fmt::format("unexpected packet type ({})", res)));
        }

        if (mustTerminateSignal || mustTerminateStop) {
            break;
        }
    }

    av_packet_free(&inputPacket);
    inputPacket = nullptr;

    if (mustTerminateSignal) {
        stop_recording();
    }


    return 0;
}

int64_t last_processed_video_pts = 0;
int64_t last_processed_audio_pts = 0;

int RecordingService::process_captured_packets_queue() {
    while (true) {
        if (capturedPacketsQueue.empty()) {
            if (!mustTerminateStop && !mustTerminateSignal)
                continue;
            break;
        };

        AVPacket *packet;
        int64_t packetPts;
        AVMediaType packetType;
        std::tie(packet, packetPts, packetType) = capturedPacketsQueue.front();
        capturedPacketsQueue.pop();

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
    }
    return 0;
}

void RecordingService::rec_stats_loop() {
    while (!mustTerminateStop && !mustTerminateSignal) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::cout << "\r Packet Queues - video: " << capturedPacketsQueue.size() << " audio: "
                  << capturedPacketsQueue.size()
                  << "Last Captured PTS - video: " << last_video_pts << " audio: "
                  << av_rescale_q(last_audio_pts, outputAudioAvcc->time_base, {1, 1000})
                  << "Last Processed PTS - video: " << last_processed_video_pts << " audio: "
                  << av_rescale_q(last_processed_audio_pts, outputAudioAvcc->time_base, {1, 1000});
    }
}

int RecordingService::start_recording() {
    // Write output file header
    if (avformat_write_header(outputAvfc, nullptr) < 0) {
        //Handle
        return -1;
    }


    // Call start_recording_loop in a new thread
    captureThread = std::thread([this]() {
        start_capture_loop();
    });

    capturedPacketsProcessThread = std::thread([this]() {
        process_captured_packets_queue();
    });

    recordingStatsThread = std::thread([this]() {
        rec_stats_loop();
    });

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
    mustTerminateStop = true;

    if (captureThread.joinable())
        captureThread.join();

    if (capturedPacketsProcessThread.joinable())
        capturedPacketsProcessThread.join();

    if (recordingStatsThread.joinable())
        recordingStatsThread.join();

    if (encode_video(0, nullptr)) return -1;
    if (encode_audio_from_buffer(0, true)) return -1;

    if (av_write_trailer(outputAvfc) < 0) {
        std::cout << "write error" << std::endl;
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
    // Initialize the LibAV devices
    avdevice_register_all();

    // Unpack device addresses
    std::string videoDeviceID, videoURL, audioDeviceID, audioURL;
    std::tie(videoDeviceID, videoURL) = unpackDeviceAddress(videoAddress);
    std::tie(audioDeviceID, audioURL) = unpackDeviceAddress(audioAddress);

    // Open A/V devices
    if (videoDeviceID == audioDeviceID) {
        inputAvfc = init_input_device(videoDeviceID, videoURL, audioURL, get_device_options(videoDeviceID));
        inputAuxAvfc = inputAvfc; // The auxiliary device context is the same because we only use one device for A/V
    } else {
        inputAvfc = init_input_device(videoDeviceID, videoURL, "", get_device_options(videoDeviceID));
        inputAuxAvfc = init_input_device(audioDeviceID, "", audioURL, get_device_options(audioDeviceID));
    }

    // Get video stream
    init_video_input_stream(inputAvfc, &inputVideoAvs, &inputVideoAvc);

    // Open video decoder
    init_input_stream_decoder(inputVideoAvs, &inputVideoAvc, &inputVideoAvcc);

    // Get audio stream
    init_audio_input_stream(inputAuxAvfc, &inputAudioAvs, &inputAudioAvc);

    // Open audio decoder
    init_input_stream_decoder(inputAudioAvs, &inputAudioAvc, &inputAudioAvcc);

    // Open output file
    outputAvfc = init_output_context_and_file(outputFilename);

    // Initialize output streams
    init_output_stream(outputAvfc, &outputVideoAvs);
    init_output_stream(outputAvfc, &outputAudioAvs);

    // Initialize output encoders
    init_video_encoder(inputAvfc, inputVideoAvs, outputVideoAvs, &outputVideoAvc, &outputVideoAvcc);
    init_audio_encoder(inputAudioAvcc, outputAudioAvs, &outputAudioAvc, &outputAudioAvcc);

    // Initialize output converters
    init_video_converter(inputVideoAvcc, outputVideoAvcc, &videoConverter);
    init_audio_converter(inputAudioAvcc, outputAudioAvcc, &audioConverter, &audioConverterBuffer);

    // Initialize signal to stop recording on sigterm
    std::signal(SIGTERM, [](int) {
        std::cout << "echo" << std::endl;
        mustTerminateSignal = true;
    });
}
