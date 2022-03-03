#ifndef PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
#define PDS_SCREEN_RECORDING_RECORDINGSERVICE_H

#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <condition_variable>
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
const int64_t OUTPUT_VIDEO_BIT_RATE = 2000000;
const int64_t OUTPUT_AUDIO_BIT_RATE = 96000;

enum RecordingStatus {
    IDLE, RECORDING, PAUSE, STOP
};

struct RecordingStats {
    RecordingStatus status;
    int64_t recordingDuration; // seconds
};

class RecordingServiceImpl {
    // --------
    // Internal
    // --------

    RecordingStatus recordingStatus;
    int64_t startTimestamp; // microseconds
    int64_t pauseTimestamp;// microseconds
    int64_t stopTimestamp;// microseconds

    std::condition_variable captureCV;
    std::mutex recordingStatusMutex;

    std::mutex videoProcessChainQueueMutex;
    std::condition_variable videoProcessChainCV;
    std::mutex audioProcessChainQueueMutex;
    std::condition_variable audioProcessChainCV;

    // -------
    // Threads
    // -------

    std::thread mainDeviceCaptureThread;
    std::thread auxDeviceCaptureThread;
    std::thread capturedVideoPacketsProcessThread;
    std::thread capturedAudioPacketsProcessThread;

    bool useControlThread;
    std::thread controlThread;

    // ------
    // Input
    // ------

    bool isAudioDisabled;

    // Input context
    std::shared_ptr<DeviceContext> mainDevice;
    std::shared_ptr<DeviceContext> auxDevice;

    // ------
    // Output
    // ------

    // Output context
    std::shared_ptr<DeviceContext> outputMuxer;

    // ----------------
    // Packet Capturers
    // ----------------

    std::unique_ptr<PacketCapturer> mainDeviceCapturer;
    std::unique_ptr<PacketCapturer> auxDeviceCapturer;

    // ---------------
    // Transcode Chain
    // ---------------

    std::unique_ptr<ProcessChain> videoTranscodeChain;
    std::unique_ptr<ProcessChain> audioTranscodeChain;

    // recording_utils.cpp
    static std::map<std::string, std::string> get_device_options(
        const std::string &deviceID,
        const RecordingConfig &config);

    static std::tuple<std::string, std::string> unpackDeviceAddress(
        const std::string &deviceAddress);

    static std::tuple<int, int, int, int, int, int> get_output_image_parameters(
        int deviceInputWidth,
        int deviceInputHeight,
        const RecordingConfig &config);

    // recording_service.cpp
    void start_capture_loop(PacketCapturer &capturer);

    void start_transcode_process(ProcessChain &transcodeChain, std::mutex& queueMutex, std::condition_variable& queueCV);

public:
    explicit RecordingServiceImpl(const RecordingConfig &config);

    void start_recording();

    void pause_recording();

    void resume_recording();

    void stop_recording();

    void wait_recording();

    RecordingStats get_recording_stats();

    ~RecordingServiceImpl() = default;
};

#endif  // PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
