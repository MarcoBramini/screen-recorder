#ifndef PDS_SCREEN_RECORDING_DEVICE_CONTEXT_H
#define PDS_SCREEN_RECORDING_DEVICE_CONTEXT_H

#include <vector>
#include <string>
#include <map>
#include <memory>
#include "ffmpeg_objects_deleter.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

class DeviceContext {
    std::unique_ptr<AVFormatContext, FFMpegObjectsDeleter> avfc;

    // These are just convenience pointers to the context main A/V streams. They follow the context lifecycle
    AVStream *videoStream;
    AVStream *audioStream;

    static std::unique_ptr<AVFormatContext, FFMpegObjectsDeleter>
    init_input_device(const std::string &deviceID, const std::string &videoURL,
                      const std::string &audioURL,
                      const std::map<std::string, std::string> &optionsMap);

    int find_main_stream(AVMediaType streamType);

    static std::unique_ptr<AVFormatContext, FFMpegObjectsDeleter>
    init_output_context(const std::string &outputFileName);

public:
    static std::shared_ptr<DeviceContext>
    init_demuxer(const std::string &deviceID, const std::string &videoURL, const std::string &audioURL,
                 const std::map<std::string, std::string> &optionsMap);


    static std::shared_ptr<DeviceContext> init_muxer(const std::string &outputFileName, bool isAudioDisabled);

    AVFormatContext *getContext() {
        return this->avfc.get();
    };

    AVStream *getVideoStream() {
        return this->videoStream;
    };

    AVStream *getAudioStream() {
        return this->audioStream;
    };

    ~DeviceContext() = default;
};


#endif //PDS_SCREEN_RECORDING_DEVICE_CONTEXT_H
