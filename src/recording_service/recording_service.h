#ifndef PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
#define PDS_SCREEN_RECORDING_RECORDINGSERVICE_H

#include <optional>
#include <string>
#include <iostream>

#include "input_streaming_context.h"
#include "output_streaming_context.h"


class RecordingService {
    InputStreamingContext *inputCtx;
    InputStreamingContext *inputAuxCtx;
    OutputStreamingContext *outputCtx;

    

    static AVFormatContext *open_input_device(const std::string& deviceID, const std::string& videoID, const std::string& audioID);

    static int open_input_stream_decoder(AVStream *inputStream, const AVCodec **inputCodec, AVCodecContext **inputCodecCtx);

    static AVFormatContext * open_output_file(const std::string &filename);

    int start_recording_loop();

public:
    bool mustTerminate;
    RecordingService(const std::string &videoInDevID, const std::string &audioInDevID,
                     const std::string &outputFilename);

    int start_recording();

    int pause_recording();

    int resume_recording();

    int stop_recording();

    static int prepare_video_encoder(InputStreamingContext *inputCtx, OutputStreamingContext *outputCtx);

    static int prepare_audio_encoder(InputStreamingContext *inputCtx, OutputStreamingContext *outputCtx);

    int encode_video(AVFrame *videoInputFrame);

    int transcode_video(InputStreamingContext *input, OutputStreamingContext *output, AVPacket *videoInputPacket,
                        AVFrame *videoInputFrame);

    int transcode_video(AVPacket *videoInputPacket, AVFrame *videoInputFrame);
};


#endif //PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
