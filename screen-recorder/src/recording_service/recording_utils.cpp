#include <fmt/core.h>
#include "recording_service_impl.h"

/// Returns the options associated to an input device.
std::map<std::string, std::string> RecordingServiceImpl::get_device_options(
        const std::string &deviceID,
        RecordingConfig config) {
    if (deviceID == "avfoundation") {
        return {{"video_size",     "1000x500"},
                {"framerate",      std::to_string(config.getFramerate())},
                {"capture_cursor", "true"}};
    }

    if (deviceID == "x11grab") {
        return {{"framerate",  std::to_string(config.getFramerate())},
                {"video_size", "1680x900"}};
    }

    if (deviceID == "pulse") {
        return {};
    }

    return {};
}

std::tuple<std::string, std::string> RecordingServiceImpl::unpackDeviceAddress(
        const std::string &deviceAddress) {
    int delimiterIndex = deviceAddress.find(':');
    std::string deviceID = deviceAddress.substr(0, delimiterIndex);
    std::string url =
            deviceAddress.substr(delimiterIndex + 1, deviceAddress.length());

    return std::make_tuple(deviceID, url);
}

inline int make_even(int n) {
    return n - n % 2;
}

// Calculates the parameters of the output image.
// Returns:
// - encoderOutputWidth, encoderOutputHeight: the real output image resolution
// - scalerOutputWidth, scalerOutputHeight: the intermediate image resolution (same as the encoder resolution for fullscreen recording)
// - cropOriginX, cropOriginY: origin of the image (0 for fullscreen recording)
std::tuple<int, int, int, int, int, int> RecordingServiceImpl::get_output_image_parameters(
        int deviceInputWidth,
        int deviceInputHeight,
        const RecordingConfig &config) {

    int encoderOutputWidth = deviceInputWidth;
    int encoderOutputHeight = deviceInputHeight;
    int scalerOutputWidth = encoderOutputWidth;
    int scalerOutputHeight = encoderOutputHeight;
    int cropOriginX = 0;
    int cropOriginY = 0;

    // Set output resolution
    if (config.getOutputResolution().has_value()) {
        auto[width, height] = config.getOutputResolution().value();
        width = make_even(width);
        height = make_even(height);

        encoderOutputWidth = width;
        encoderOutputHeight = height;
        scalerOutputWidth = width;
        scalerOutputHeight = height;
    }

    if (config.getCaptureRegion().has_value()) {
        auto[x, y, width, height] = config.getCaptureRegion().value();

        encoderOutputWidth = make_even(width);
        encoderOutputHeight = make_even(height);
        cropOriginX = make_even(x);
        cropOriginY = make_even(y);

    }

    return {encoderOutputWidth, encoderOutputHeight, scalerOutputWidth, scalerOutputHeight, cropOriginX, cropOriginY};
}