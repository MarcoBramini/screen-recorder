#include <fmt/core.h>
#include "recording_service.h"

/// Returns the options associated to an input device.
std::map<std::string, std::string> RecordingService::get_device_options(const std::string &deviceID) {
    if (deviceID == "avfoundation") {
        return {
                {"video_size",     "1000x500"},
                {"framerate",      std::to_string(OUTPUT_VIDEO_FRAME_RATE)},
                {"capture_cursor", "true"}};
    }

    if (deviceID == "x11grab") {
        return {{"framerate",  std::to_string(OUTPUT_VIDEO_FRAME_RATE)},
                {"video_size", "1680x900"}};
    }

    if (deviceID == "pulse") {
        return {};
    }

    return {};
}


std::tuple<std::string, std::string> RecordingService::unpackDeviceAddress(const std::string &deviceAddress) {
    int delimiterIndex = deviceAddress.find(':');
    std::string deviceID = deviceAddress.substr(0, delimiterIndex);
    std::string url = deviceAddress.substr(delimiterIndex + 1, deviceAddress.length());

    return std::make_tuple(deviceID, url);
}
