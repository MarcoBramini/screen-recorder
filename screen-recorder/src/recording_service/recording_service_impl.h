#ifndef PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
#define PDS_SCREEN_RECORDING_RECORDINGSERVICE_H

#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include "recording_config.h"
#include "device_context.h"
#include "packet_capturer/packet_capturer.h"
#include "process_chain/process_chain.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}


// Settings
const AVSampleFormat OUTPUT_AUDIO_SAMPLE_FMT = AV_SAMPLE_FMT_FLTP;
const AVPixelFormat OUTPUT_VIDEO_PIXEL_FMT = AV_PIX_FMT_YUV420P;
const int64_t OUTPUT_VIDEO_BIT_RATE = 1500000;
const int64_t OUTPUT_AUDIO_BIT_RATE = 96000;

enum RecordingStatus {
    IDLE, RECORDING, PAUSE, STOP
};

class RecordingServiceImpl {
    // ------
    // Status
    // ------
    RecordingStatus recordingStatus;
    int64_t pauseTimestamp;

    // -------
    // Threads
    // -------

    std::thread videoCaptureThread;
    std::thread audioCaptureThread;
    std::thread capturedVideoPacketsProcessThread;
    std::thread capturedAudioPacketsProcessThread;
    std::thread recordingStatsThread;
    std::thread controlThread;

    // ------
    // Input
    // ------

    // Input context
    DeviceContext *mainDevice;
    DeviceContext *auxDevice;

    // ------
    // Output
    // ------

    // Output context
    DeviceContext *outputMuxer;

    // ----------------
    // Packet Capturers
    // ----------------

    PacketCapturer *mainDeviceCapturer;
    PacketCapturer *auxDeviceCapturer;

    // ---------------
    // Transcode Chain
    // ---------------

    ProcessChain *videoTranscodeChain;
    ProcessChain *audioTranscodeChain;

    // recording_utils.cpp
    static std::map<std::string, std::string> get_device_options(
            const std::string &deviceID,
            RecordingConfig config);

    static std::tuple<std::string, std::string> unpackDeviceAddress(
            const std::string &deviceAddress);

    static std::tuple<int, int, int, int, int, int> get_output_image_parameters(
            int deviceInputWidth,
            int deviceInputHeight,
            const RecordingConfig &config);

    // recording_service.cpp
    void start_capture_loop(PacketCapturer *capturer);

    void start_transcode_process(ProcessChain *transcodeChain);

public:
    explicit RecordingServiceImpl(const RecordingConfig& config);

    int start_recording();

    int pause_recording();

    int resume_recording();

    int stop_recording();

    void rec_stats_loop();

    void wait_recording();

    ~RecordingServiceImpl() {
        if (mainDevice != auxDevice) {
            delete auxDevice;
            delete auxDeviceCapturer;
        }
        delete mainDevice;
        delete mainDeviceCapturer;

        delete outputMuxer;

        delete videoTranscodeChain;
        delete audioTranscodeChain;
    };
};

#endif  // PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
