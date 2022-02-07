#include "iostream"
#include <vector>
#include <thread>
#include <chrono>
#include <fmt/core.h>

#include "src/device_service/device_service.h"
#include "src/device_service/input_device.h"
#include "src/recording_service/recording_service.h"

int main() {
    // List all the available input devices
    std::vector<InputDeviceVideo> videoDevices = DeviceService::get_input_video_devices();
    std::vector<InputDeviceAudio> audioDevices = DeviceService::get_input_audio_devices();

    std::cout << "Video devices:" << std::endl;
    int i;
    for (i = 0; i < videoDevices.size(); i++) {
        std::cout << i << " -> " << videoDevices[i].toString() << std::endl;
    }
    std::cout << "Audio devices:" << std::endl;
    for (i = 0; i < audioDevices.size(); i++) {
        std::cout << i << " -> " << audioDevices[i].toString() << std::endl;
    }

    // Ask user what device to capture
    int videoDeviceID, audioDeviceID;
    std::cout << "Insert the index of the video device:" << std::endl;
    scanf("%d", &videoDeviceID);
    std::cout << "Insert the index of the audio device:" << std::endl;
    scanf("%d", &audioDeviceID);

    std::string videoAddress = fmt::format("{}:{}", videoDevices[videoDeviceID].getDeviceID(),
                                           videoDevices[videoDeviceID].getURL());
    std::string audioAddress = fmt::format("{}:{}", audioDevices[audioDeviceID].getDeviceID(),
                                           audioDevices[audioDeviceID].getURL());

    // Start recording
    RecordingService rs = RecordingService(videoAddress, audioAddress, "output.mp4");
    rs.start_recording();

    rs.wait_recording();

    return 0;
}