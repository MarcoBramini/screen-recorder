#ifndef PDS_SCREEN_RECORDING_DEVICE_SERVICE_H
#define PDS_SCREEN_RECORDING_DEVICE_SERVICE_H

#include "../src/device_service/input_device.h"
#include <vector>

class DeviceService{
public:
    static std::vector<InputDeviceVideo> get_input_video_devices();
    static std::vector<InputDeviceAudio> get_input_audio_devices();
};

#endif //PDS_SCREEN_RECORDING_DEVICE_SERVICE_H
