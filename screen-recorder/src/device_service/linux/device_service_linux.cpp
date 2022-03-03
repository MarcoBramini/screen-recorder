#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <dirent.h>
#include <vector>
#include "../../../include/device_service.h"

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
}

static const std::string DEVICE_ID_X11GRAB = "x11grab";
static const std::string DEVICE_ID_PULSE = "pulse";

std::vector<InputDeviceVideo> DeviceService::get_input_video_devices() {
  std::vector<InputDeviceVideo> devices;
  Display* display;
  display = XOpenDisplay(nullptr);

  if (display == nullptr) {
    return devices;
  }
  int monitorCnt;
  XRRMonitorInfo* xMonitors =
      XRRGetMonitors(display, DefaultRootWindow(display), false, &monitorCnt);

  if (monitorCnt == 0) {
    return devices;
  }
  std::string screenName = "Monitor";

  for (int monitorIdx = 0; monitorIdx < monitorCnt; monitorIdx++) {
    std::string name = screenName + std::to_string(monitorIdx + 1);
    int x = xMonitors[monitorIdx].x;
    int y = xMonitors[monitorIdx].y;
    std::string id = ":0.0+" + std::to_string(x) + "," + std::to_string(y);
    int width = xMonitors[monitorIdx].width;
    int height = xMonitors[monitorIdx].height;
    int primary = xMonitors[monitorIdx].primary;
    char* port = XGetAtomName(display, xMonitors[monitorIdx].name);
    InputDeviceVideo deviceVideo(
        id, DEVICE_ID_X11GRAB, name, static_cast<float>(x),
        static_cast<float>(y), static_cast<float>(width),
        static_cast<float>(height), primary, std::string(port));
    devices.push_back(deviceVideo);
    XFree(port);
  }

  XRRFreeMonitors(xMonitors);
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

  for (int i = 0; i < l->nb_devices; i++) {
    InputDeviceAudio deviceAudio(l->devices[i]->device_name, DEVICE_ID_PULSE,
                                 l->devices[i]->device_description);
    devices.push_back(deviceAudio);
  }
  avdevice_free_list_devices(&l);
  return devices;
}

PermissionsStatus DeviceService::check_permissions() {
  return {.screenCaptureAllowed = true,
          .microphoneCaptureAllowed = true,
          .cameraCaptureAllowed = true};
}

void DeviceService::setup_screen_capture_permission(
    std::function<void(bool)> onComplete) {}

void DeviceService::setup_microphone_usage_permission(
    std::function<void(bool)> onComplete) {}

void DeviceService::setup_camera_usage_permission(
    std::function<void(bool)> onComplete) {}