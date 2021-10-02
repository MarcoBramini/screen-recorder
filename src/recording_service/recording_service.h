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


    AVFormatContext *open_input_device(std::string deviceID, std::string url);

    int open_input_stream_decoder(AVStream *inputStream, const AVCodec **inputCodec, AVCodecContext **inputCodecCtx);

    OutputStreamingContext open_output_file(const std::string &filename);

    int start_recording_loop(InputStreamingContext inputCtx,
                             std::optional<InputStreamingContext> inputAuxCtx,
                             OutputStreamingContext outputCtx);

public:
    RecordingService(const std::string &videoInDevID, const std::string &audioInDevID,
                     const std::string &outputFilename);

    int start_recording();

    int pause_recording();

    int resume_recording();

    int stop_recording();
};


#endif //PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
