//
// Created by Marco Bramini on 01/11/21.
//

#ifndef PDS_SCREEN_RECORDING_DEVICE_CONTEXT_H
#define PDS_SCREEN_RECORDING_DEVICE_CONTEXT_H

#include <vector>
#include <string>
#include <map>

#include "stream_context.h"

extern "C" {
#include "libavformat/avformat.h"
};


class DeviceContext {
    AVFormatContext *avfc;
    std::optional<StreamContext> videoStream;
    std::optional<StreamContext> audioStream;

    static AVFormatContext *init_input_device(const std::string &deviceID, const std::string &videoURL,
                                              const std::string &audioURL,
                                              const std::map<std::string, std::string> &optionsMap);

    int find_main_stream(AVMediaType streamType);

    static AVFormatContext *init_output_context(const std::string &outputFileName);

public:
    static DeviceContext
    init_demuxer(const std::string &deviceID, const std::string &videoURL, const std::string &audioURL,
                 const std::map<std::string, std::string> &optionsMap);


    static DeviceContext init_muxer(const std::string &outputFileName);

    AVFormatContext *getContext() { return this->avfc; };

    std::optional<StreamContext> getVideoStream() { return this->videoStream; };

    std::optional<StreamContext> getAudioStream() { return this->audioStream; };
};


#endif //PDS_SCREEN_RECORDING_DEVICE_CONTEXT_H
