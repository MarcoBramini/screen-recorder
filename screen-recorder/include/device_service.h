#ifndef PDS_SCREEN_RECORDING_DEVICE_SERVICE_H
#define PDS_SCREEN_RECORDING_DEVICE_SERVICE_H

#include <functional>
#include <vector>
#include "../src/device_service/input_device.h"
#include "../src/device_service/permission_status.h"

class DeviceService {
 public:
  static std::vector<InputDeviceVideo> get_input_video_devices();
  static std::vector<InputDeviceAudio> get_input_audio_devices();

  static PermissionsStatus check_permissions();
  static void setup_screen_capture_permission(
      std::function<void(bool)> onComplete);
  static void setup_microphone_usage_permission(
      std::function<void(bool)> onComplete);
  static void setup_camera_usage_permission(
      std::function<void(bool)> onComplete);
};

#endif  // PDS_SCREEN_RECORDING_DEVICE_SERVICE_H
