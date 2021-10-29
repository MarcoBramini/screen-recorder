#include <fmt/core.h>
#include "recording_service.h"

/// Returns the options associated to an input device.
std::map<std::string, std::string> RecordingService::get_device_options(const std::string &deviceID) {
    if (deviceID == "avfoundation") {
        return {{"framerate",      "30"},
                {"capture_cursor", "true"}};
    }

    if (deviceID == "x11grab") {
        return {{"framerate",      "25"},
                {"video_size", "1000x500"}};
    }

    if (deviceID == "pulse") {
        return {};
    }

    return {};
}

std::string RecordingService::build_error_message(const std::string &methodName,
                                                  const std::map<std::string, std::string> &methodParams,
                                                  const std::string &errorDescription) {
    // Format method params to string
    std::string params;
    for (const auto &param: methodParams) {
        params.append(fmt::format("{}:{}, ", param.first, param.second));
    }
    // Trim last comma and space
    params = params.substr(0, params.length() - 2);

    return fmt::format("{}({}): {}", methodName, params, errorDescription);
}

std::tuple<std::string, std::string> RecordingService::unpackDeviceAddress(const std::string &deviceAddress) {
    int delimiterIndex = deviceAddress.find(':');
    std::string deviceID = deviceAddress.substr(0, delimiterIndex);
    std::string url = deviceAddress.substr(delimiterIndex + 1, deviceAddress.length());

    return std::make_tuple(deviceID, url);
}

std::string RecordingService::unpackAVError(int avErrorCode){
    char a[AV_ERROR_MAX_STRING_SIZE] = { 0 };
    return av_make_error_string(a, AV_ERROR_MAX_STRING_SIZE, avErrorCode);
}