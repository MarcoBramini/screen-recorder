#include "iostream"
#include <vector>
#include <thread>
#include <chrono>

#include "src/device_service/device_service.h"
#include "src/device_service/input_device.h"
#include "src/recording_service/recording_service.h"

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

    RecordingService rs = RecordingService("avfoundation:1", "avfoundation:0", "output.mp4");
    rs.start_recording();

    std::this_thread::sleep_for(std::chrono::seconds(10));
    rs.stop_recording();


    return 0;
}