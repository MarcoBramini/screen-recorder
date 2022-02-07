#ifndef PDS_SCREEN_RECORDING_DEVICE_CONTEXT_H
#define PDS_SCREEN_RECORDING_DEVICE_CONTEXT_H

#include <vector>
#include <string>
#include <map>

extern "C" {
#include "libavformat/avformat.h"
};


class DeviceContext {
    AVFormatContext *avfc;
    AVStream *videoStream;
    AVStream *audioStream;

    static AVFormatContext *init_input_device(const std::string &deviceID, const std::string &videoURL,
                                              const std::string &audioURL,
                                              const std::map<std::string, std::string> &optionsMap);

    int find_main_stream(AVMediaType streamType);

    static AVFormatContext *init_output_context(const std::string &outputFileName);

public:
    static DeviceContext *
    init_demuxer(const std::string &deviceID, const std::string &videoURL, const std::string &audioURL,
                 const std::map<std::string, std::string> &optionsMap);


    static DeviceContext *init_muxer(const std::string &outputFileName);

    AVFormatContext *getContext() { return this->avfc; };

    AVStream *getVideoStream() { return this->videoStream; };

    AVStream *getAudioStream() { return this->audioStream; };

    ~DeviceContext() {
        avformat_close_input(&avfc);
        avformat_free_context(avfc);
    };
};


#endif //PDS_SCREEN_RECORDING_DEVICE_CONTEXT_H
