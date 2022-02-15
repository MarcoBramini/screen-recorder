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

  std::string videoAddress =
      fmt::format("{}:{}", videoDevices[videoDeviceID].getDeviceID(),
                  videoDevices[videoDeviceID].getURL());
  std::string audioAddress =
      fmt::format("{}:{}", audioDevices[audioDeviceID].getDeviceID(),
                  audioDevices[audioDeviceID].getURL());

  // Start recording
  std::tuple<int, int, int, int> cropWindow = {0, 0, 5000, 2000};
  RecordingService rs = RecordingService({
      .videoAddress = videoAddress,
      .audioAddress = audioAddress,
      .outputFilename = "output.mp4",
      .rescaleValue = 3,
      .framerate = 30,
      //.cropWindow = cropWindow,
  });
  rs.start_recording();

  rs.wait_recording();

  return 0;
}