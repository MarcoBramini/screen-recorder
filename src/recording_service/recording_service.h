#ifndef PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
#define PDS_SCREEN_RECORDING_RECORDINGSERVICE_H

#include <optional>
#include <string>
#include <iostream>
#include <thread>
#include <queue>
#include <map>

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
const int64_t OUTPUT_VIDEO_BIT_RATE = 1000000;
const int64_t OUTPUT_AUDIO_BIT_RATE = 96000;
const int OUTPUT_HEIGHT = 1080;
const int OUTPUT_WIDTH = 1920;

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
    std::thread capturedPacketsProcessThread;
    std::thread recordingStatsThread;
    std::thread controlThread;

    // ----------------
    // Processing queue
    // ----------------

    std::queue<std::tuple<AVPacket *, int64_t, AVMediaType>> capturedPacketsQueue;

    // ------
    // Input
    // ------

    // Input context
    AVFormatContext *inputAvfc;
    AVFormatContext *inputAuxAvfc;

    // Input video properties
    AVCodec *inputVideoAvc;
    AVStream *inputVideoAvs;
    AVCodecContext *inputVideoAvcc;

    // Input audio properties
    AVCodec *inputAudioAvc;
    AVStream *inputAudioAvs;
    AVCodecContext *inputAudioAvcc;

    // ----------
    // Converters
    // ----------

    // Video converter
    SwsContext *videoConverter;

    // Audio converter
    SwrContext *audioConverter;
    AVAudioFifo *audioConverterBuffer;

    // ------
    // Output
    // ------

    // Output context
    AVFormatContext *outputAvfc;

    // Output video properties
    const AVCodec *outputVideoAvc;
    AVStream *outputVideoAvs;
    AVCodecContext *outputVideoAvcc;

    // Output audio properties
    const AVCodec *outputAudioAvc;
    AVStream *outputAudioAvs;
    AVCodecContext *outputAudioAvcc;

    // recording_utils.cpp
    static std::map<std::string, std::string> get_device_options(const std::string &deviceID);

    static std::string build_error_message(const std::string &methodName,
                                           const std::map<std::string, std::string> &methodParams,
                                           const std::string &errorDescription);

    static std::tuple<std::string, std::string> unpackDeviceAddress(const std::string &deviceAddress);

    static std::string unpackAVError(int avErrorCode);

    // recording_init.cpp
    static AVFormatContext *
    init_input_device(const std::string &deviceID, const std::string &videoURL, const std::string &audioURL,
                      const std::map<std::string, std::string> &optionsMap);

    static void
    init_video_input_stream(AVFormatContext *inputAvfc, AVStream **inputVideoAvs, AVCodec **inputVideoAvc);

    static void
    init_audio_input_stream(AVFormatContext *inputAvfc, AVStream **inputAudioAvs, AVCodec **inputAudioAvc);

    static void
    init_input_stream_decoder(AVStream *inputAvs, AVCodec **inputAvc, AVCodecContext **inputAvcc);

    static AVFormatContext *init_output_context_and_file(const std::string &outputFileName);

    static void init_output_stream(AVFormatContext *outputStream, AVStream **outputVideoAvs);

    static void
    init_video_converter(AVCodecContext *inputVideoAvcc, AVCodecContext *outputVideoAvcc, SwsContext **videoConverter);

    static void
    init_audio_converter(AVCodecContext *inputAudioAvcc, AVCodecContext *outputAudioAvcc, SwrContext **audioConverter,
                         AVAudioFifo **audioConverterBuffer);

    static void init_video_encoder(AVFormatContext *inputAvfc, AVStream *inputVideoAvs, AVStream *outputVideoAvs,
                                   const AVCodec **outputVideoAvc, AVCodecContext **outputVideoAvcc);

    static void
    init_audio_encoder(AVCodecContext *inputAudioAvcc, AVStream *outputAudioAvs, const AVCodec **outputAudioAvc,
                       AVCodecContext **outputAudioAvcc);

    // recording_service.cpp
    int start_capture_loop(AVFormatContext * inputAvfc);

    int process_captured_packets_queue();

    int encode_audio_from_buffer(int64_t framePts, bool shouldFlush);

    int encode_video(int64_t framePts, AVFrame *videoInputFrame);

    int transcode_video(AVPacket *videoInputPacket, int64_t packetPts);

    int transcode_audio(AVPacket *audioInputPacket, int64_t packetPts);

    int convert_video(AVFrame *videoInputFrame, AVFrame *videoOutputFrame);

    int convert_audio(AVFrame *audioInputFrame);

public:
    RecordingService(const std::string &videoAddress, const std::string &audioAddress,
                     const std::string &outputFilename);

    int start_recording();

    int pause_recording();

    int resume_recording();

    int stop_recording();

    void enqueue_video_packet(AVPacket *inputVideoPacket);

    void enqueue_audio_packet(AVPacket *inputAudioPacket);

    void rec_stats_loop();
};


#endif //PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
