//
// Created by Marco Bramini on 16/09/21.
//

#include "../device_service.h"
#include "avfoundation_bridge.h"

#include <vector>

std::vector<InputDeviceVideo> DeviceService::get_input_video_devices() {
    std::vector<InputDeviceVideo> devices;
    avfoundation_list_video_devices(&devices);
    return devices;
}

std::vector<InputDeviceAudio> DeviceService::get_input_audio_devices() {
    std::vector<InputDeviceAudio> devices;
    avfoundation_list_audio_devices(&devices);
    return devices;
}
