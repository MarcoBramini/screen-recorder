//
// Created by Marco Bramini on 16/09/21.
//

#include "../device_service.h"
#include <vector>
#include <dirent.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
}

std::vector<InputDeviceVideo> DeviceService::get_input_video_devices() {
    std::vector<InputDeviceVideo> devices;
    Display *display;
    display = XOpenDisplay(nullptr);

    if(display == nullptr) {
        return devices;
    }
    int monitorCnt;
    XRRMonitorInfo *xMonitors = XRRGetMonitors(display, DefaultRootWindow(display), false, &monitorCnt);

    if(monitorCnt == 0) {
        return devices;
    }
    std::string screenName = "Monitor";

    for (int monitorIdx = 0; monitorIdx < monitorCnt; monitorIdx++) {
        std::string name = screenName + std::to_string(monitorIdx+1);
        int x = xMonitors[monitorIdx].x;
        int y = xMonitors[monitorIdx].y;
        std::string id = "0.0+" + std::to_string(x) + "," + std::to_string(y);
        int width = xMonitors[monitorIdx].width;
        int height = xMonitors[monitorIdx].height;
        int primary = xMonitors[monitorIdx].primary;
        std::string port = XGetAtomName( display, xMonitors[monitorIdx].name);
        InputDeviceVideo deviceVideo(id,name,static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height) ,primary,port);
        devices.push_back(deviceVideo);
    }
    return devices;
}

std::vector<InputDeviceAudio> DeviceService::get_input_audio_devices() {
    std::vector<InputDeviceAudio> devices;
    avdevice_register_all();
    AVInputFormat* fmt = av_find_input_format("pulse");
    AVDeviceInfoList* l;
    int r = avdevice_list_input_sources(fmt, NULL, NULL, &l);

    if (r != 0) {
        return devices;
    }

    for (int i = 0;i < l->nb_devices;i++) {
        InputDeviceAudio deviceAudio(l->devices[i]->device_name, l->devices[i]->device_description);
        devices.push_back(deviceAudio);
    }
    avdevice_free_list_devices(&l);
    return devices;
}
