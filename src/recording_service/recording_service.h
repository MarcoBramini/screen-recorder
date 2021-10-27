#ifndef PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
#define PDS_SCREEN_RECORDING_RECORDINGSERVICE_H

#include <optional>
#include <string>
#include <iostream>
#include <thread>
#include <queue>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
}

class RecordingService {

    std::thread captureThread;
    std::thread audioProcessThread;
    std::thread videoProcessThread;

    std::queue<std::tuple<AVPacket *, int64_t>> videoPacketsQueue;
    std::queue<std::tuple<AVPacket *, int64_t>> audioPacketsQueue;

    // Input context
    AVFormatContext *inputAvfc;
    AVFormatContext *inputAuxAvfc;

    // Input video properties
    const AVCodec *inputVideoAvc;
    AVStream *inputVideoAvs;
    AVCodecContext *inputVideoAvcc;
    int inputVideoIndex;

    // Input audio properties
    const AVCodec *inputAudioAvc;
    AVStream *inputAudioAvs;
    AVCodecContext *inputAudioAvcc;
    int inputAudioIndex;

    // Audio converter
    SwrContext *audioConverter;
    AVAudioFifo *audioBuffer;

    // Output context
    AVFormatContext *outputAvfc;
    std::string filename;

    // Output video properties
    const AVCodec *outputVideoAvc;
    AVStream *outputVideoAvs;
    AVCodecContext *outputVideoAvcc;

    // Output audio properties
    const AVCodec *outputAudioAvc;
    AVStream *outputAudioAvs;
    AVCodecContext *outputAudioAvcc;


    static AVFormatContext *
    open_input_device(const std::string &deviceID, const std::string &videoID, const std::string &audioID);

    static int
    open_input_stream_decoder(AVStream *inputStream, const AVCodec **inputCodec, AVCodecContext **inputCodecCtx);

    static AVFormatContext *open_output_file(const std::string &filename);

    int prepare_audio_encoder();

    int prepare_audio_converter();

    int prepare_video_encoder();

    int start_capture_loop();

    int convert_audio(AVFrame *audioInputFrame);

public:
    RecordingService(const std::string &videoInDevID, const std::string &audioInDevID,
                     const std::string &outputFilename);

    int start_recording();

    int pause_recording();

    int resume_recording();

    int stop_recording();


    int process_video_queue();

    int process_audio_queue();

    int encode_audio_from_buffer(int64_t framePts, bool shouldFlush);

    int encode_video(int64_t framePts, AVFrame *videoInputFrame);

    int transcode_video(AVPacket *videoInputPacket, int64_t packetPts);

    int transcode_audio(AVPacket *audioInputPacket, int64_t packetPts);
};


#endif //PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
