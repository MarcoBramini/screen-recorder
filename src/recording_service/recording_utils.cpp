#include <fmt/core.h>
#include "recording_service.h"

/// Returns the options associated to an input device.
std::map<std::string, std::string> RecordingService::get_device_options(
    const std::string& deviceID,
    RecordingConfig config) {
  if (deviceID == "avfoundation") {
    return {{"video_size", "1000x500"},
            {"framerate", std::to_string(config.framerate)},
            {"capture_cursor", "true"}};
  }

  if (deviceID == "x11grab") {
    return {{"framerate", std::to_string(config.framerate)},
            {"video_size", "1680x900"}};
  }

  if (deviceID == "pulse") {
    return {};
  }

  return {};
}

std::tuple<std::string, std::string> RecordingService::unpackDeviceAddress(
    const std::string& deviceAddress) {
  int delimiterIndex = deviceAddress.find(':');
  std::string deviceID = deviceAddress.substr(0, delimiterIndex);
  std::string url =
      deviceAddress.substr(delimiterIndex + 1, deviceAddress.length());

  return std::make_tuple(deviceID, url);
}

inline int make_even(int n) {
  return n - n % 2;
}

std::tuple<int, int> RecordingService::get_scaled_resolution(int width,
                                                             int height,
                                                             float scale) {
  return {make_even((int)width / scale), make_even((int)height / scale)};
}

std::tuple<int, int, int, int> RecordingService::get_output_window(
    int width,
    int height,
    RecordingConfig config) {
  int outputWidth = width;
  int outputHeight = height;
  int originX = 0;
  int originY = 0;

  if (config.cropWindow.has_value()) {
    auto [x1, y1, x2, y2] = config.cropWindow.value();
    outputWidth = x2 - x1;
    outputHeight = y2 - y1;
    originX = x1;
    originY = y1;
  }

  return {make_even((int)outputWidth / config.rescaleValue),
          make_even((int)outputHeight / config.rescaleValue),
          make_even((int)originX / config.rescaleValue),
          make_even((int)originY / config.rescaleValue)};
}
