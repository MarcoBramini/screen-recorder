#include <device_service.h>
#include <fmt/core.h>
#include <recording_service.h>

#include <chrono>
#include <thread>
#include <vector>

#include "iostream"

int main() {
  // List all the available input devices
  std::vector<InputDeviceVideo> videoDevices =
      DeviceService::get_input_video_devices();
  std::vector<InputDeviceAudio> audioDevices =
      DeviceService::get_input_audio_devices();

    RecordingConfig config;
    config.setUseControlThread(true);
    config.setOutputDir(".");
    config.setFramerate(30);
    //config.setCaptureRegion(100,100,300,300);

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

  config.setVideoAddress(videoDevices[videoDeviceID].getDeviceAddress());
  config.setAudioAddress(audioDevices[audioDeviceID].getDeviceAddress());

  // Start recording
  RecordingService rs = RecordingService(config);
  rs.start_recording();

  rs.wait_recording();

  return 0;
}
