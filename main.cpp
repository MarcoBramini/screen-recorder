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
    for (i = 0; i<videoDevices.size();i++) {
        std::cout <<i<<":"<< videoDevices[i].toString() << std::endl;
    }
    std::cout << "Audio devices:" << std::endl;
    for (i = 0; i<audioDevices.size();i++) {
        std::cout <<i<<":"<< audioDevices[i].toString() << std::endl;
    }

    int videoDeviceID, audioDeviceID;
    std::cout<<"Insert the index of the video device:"<<std::endl;
    scanf("%d",&videoDeviceID);
    std::cout<<"Insert the index of the audio device:"<<std::endl;
    scanf("%d",&audioDeviceID);
    RecordingService rs = RecordingService(fmt::format("x11grab::{}",videoDevices[videoDeviceID].getID()), fmt::format("pulse:{}", audioDevices[audioDeviceID].getID()), "output.mp4");
    rs.start_recording();

    std::this_thread::sleep_for(std::chrono::seconds(10));
    rs.stop_recording();


    return 0;
}