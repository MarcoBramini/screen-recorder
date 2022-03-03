#include "../../../include/device_service.h"
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

PermissionsStatus DeviceService::check_permissions() {
  bool screenCaptureAllowed = check_screen_capture_permissions();
  bool microphoneCaptureAllowed = check_microphone_capture_permissions();
  bool cameraCaptureAllowed = check_camera_capture_permissions();

  return {.screenCaptureAllowed = screenCaptureAllowed,
          .microphoneCaptureAllowed = microphoneCaptureAllowed,
          .cameraCaptureAllowed = cameraCaptureAllowed};
}

void DeviceService::setup_screen_capture_permission(
    std::function<void(bool)> onComplete) {
  request_screen_capture_permissions(onComplete);
}

void DeviceService::setup_microphone_usage_permission(
    std::function<void(bool)> onComplete) {
  request_microphone_capture_permissions(onComplete);
}

void DeviceService::setup_camera_usage_permission(
    std::function<void(bool)> onComplete) {
  request_camera_capture_permissions(onComplete);
}
