#include <vector>
#include <stdint.h>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
};

#include "iostream"
#include "src/device_service/device_service.h"
#include "src/device_service/input_device.h"
int main() {
    // List all the available input devices
    std::vector<InputDeviceVideo> videoDevices = DeviceService::get_input_video_devices();
    std::vector<InputDeviceAudio> audioDevices = DeviceService::get_input_audio_devices();

    std::cout << "Video devices:" << std::endl;
    for (InputDeviceVideo inDev : videoDevices) {
        inDev.toString();
    }
    std::cout << "Audio devices:" << std::endl;
    for (InputDeviceAudio inDev : audioDevices) {
        inDev.toString();
    }

    // Select the input devices to capture
    auto videoInDevID = "1";
    auto audioInDevID = "0";

    // Select recording area
    //

    // Select output file
    auto outputFile = "out.mkv";

    // Start recording

    // Pause recording

    // Stop recording


    return 0;
}


/*
 * AVFormatContext *pFormatCtx = avformat_alloc_context();

    AVDictionary *options = NULL;
    av_dict_set(&options, "list_devices", "true", 0);
    av_dict_set_int(&options, "framerate",30,0);
    av_dict_set(&options, "pixel_format","uyvy422",0);

    avdevice_register_all();
    const AVInputFormat *fmt = av_find_input_format("avfoundation");
    AVDeviceInfoList *l;

    int r = avformat_open_input(&pFormatCtx, "0", fmt, &options);
    if (r != 0) {
        av_log(NULL, AV_LOG_ERROR, "%s\n", av_err2str(r));
    }
 */