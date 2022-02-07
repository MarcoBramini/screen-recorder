#ifndef PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
#define PDS_SCREEN_RECORDING_RECORDINGSERVICE_H

#include <optional>
#include <string>
#include <iostream>
#include <thread>
#include <queue>
#include <map>
#include "device_context.h"
#include "process_chain/process_chain.h"

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
#include <libswscale/swscale.h>
}

// Settings
const AVSampleFormat OUTPUT_AUDIO_SAMPLE_FMT = AV_SAMPLE_FMT_FLTP;
const AVPixelFormat OUTPUT_VIDEO_PIXEL_FMT = AV_PIX_FMT_YUV420P;
const int64_t OUTPUT_VIDEO_BIT_RATE = 1500000;
const int64_t OUTPUT_AUDIO_BIT_RATE = 96000;
const int OUTPUT_HEIGHT = 1080;
const int OUTPUT_WIDTH = 1920;
const int OUTPUT_VIDEO_FRAME_RATE = 30;

enum RecordingStatus {
    IDLE,
    RECORDING,
    PAUSE,
    STOP
};

class RecordingService {

    // ------
    // Status
    // ------
    RecordingStatus recordingStatus;

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

    // ---------------
    // Transcode Chain
    // ---------------

    ProcessChain *videoTranscodeChain;
    ProcessChain *audioTranscodeChain;

    // recording_utils.cpp
    static std::map<std::string, std::string> get_device_options(const std::string &deviceID);

    static std::tuple<std::string, std::string> unpackDeviceAddress(const std::string &deviceAddress);

    // recording_service.cpp
    int start_capture_loop(DeviceContext *inputDevice);

    void start_transcode_process(ProcessChain *transcodeChain);

public:
    RecordingService(const std::string &videoAddress, const std::string &audioAddress,
                     const std::string &outputFilename);

    int start_recording();

    int pause_recording();

    int resume_recording();

    int stop_recording();

    void enqueue_video_packet(DeviceContext *inputDevice, AVPacket *inputVideoPacket);

    void enqueue_audio_packet(DeviceContext *inputDevice, AVPacket *inputAudioPacket);

    void rec_stats_loop();

    void wait_recording();

    ~RecordingService() {
        if (mainDevice != auxDevice) {
            delete auxDevice;
        }
        delete mainDevice;
        delete outputMuxer;

        delete videoTranscodeChain;
        delete audioTranscodeChain;
    };
};


#endif //PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
