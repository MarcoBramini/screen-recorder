#include <fmt/core.h>
#include "recording_service_impl.h"

/// Returns the default options associated to an input device.
std::map<std::string, std::string> RecordingServiceImpl::get_device_options(
        const std::string &deviceID,
        const RecordingConfig &config) {
    if (deviceID == "avfoundation") {
        return {{"pixel_format",   "uyvy422"},
                {"framerate",      std::to_string(config.getFramerate())},
                {"capture_cursor", "true"}};
    }

    if (deviceID == "x11grab") {
        return {{"framerate", std::to_string(config.getFramerate())}};
    }

    if (deviceID == "pulse") {
        return {};
    }

    return {};
}

/// Unpacks a deviceAddress.
/// The accepted device address format is: "{deviceID}:{url}"
std::tuple<std::string, std::string> RecordingServiceImpl::unpackDeviceAddress(
        const std::string &deviceAddress) {
    size_t delimiterIndex = deviceAddress.find(':');
    std::string deviceID = deviceAddress.substr(0, delimiterIndex);
    std::string url =
            deviceAddress.substr(delimiterIndex + 1, deviceAddress.length());

    return std::make_tuple(deviceID, url);
}

inline int make_even(int n) {
    return n - n % 2;
}

/// Calculates the parameters of the output image.
/// Returns:
/// - encoderOutputWidth, encoderOutputHeight: the real output image resolution
/// - scalerOutputWidth, scalerOutputHeight: the intermediate image resolution
/// (same as the encoder resolution for fullscreen recording)
/// - cropOriginX, cropOriginY: origin of the image (0 for fullscreen recording)
std::tuple<int, int, int, int, int, int>
RecordingServiceImpl::get_output_image_parameters(
        int deviceInputWidth,
        int deviceInputHeight,
        const RecordingConfig &config) {
    int encoderOutputWidth = deviceInputWidth;
    int encoderOutputHeight = deviceInputHeight;
    int scalerOutputWidth = encoderOutputWidth;
    int scalerOutputHeight = encoderOutputHeight;
    int cropOriginX = 0;
    int cropOriginY = 0;

    double scalingFactor = 1;

    if (config.getOutputResolution().has_value()) {
        auto[width, height, factor] = config.getOutputResolution().value();
        scalingFactor = factor;

        encoderOutputWidth = make_even((int) (encoderOutputWidth * scalingFactor));
        encoderOutputHeight = make_even((int) (encoderOutputHeight * scalingFactor));
        scalerOutputWidth = encoderOutputWidth;
        scalerOutputHeight = encoderOutputHeight;
    }

    if (config.getCaptureRegion().has_value()) {
        auto[x, y, width, height] = config.getCaptureRegion().value();

        encoderOutputWidth = make_even((int) (width * scalingFactor));
        encoderOutputHeight = make_even((int) (height * scalingFactor));
        cropOriginX = make_even((int) (x * scalingFactor));
        cropOriginY = make_even((int) (y * scalingFactor));
    }

    return {encoderOutputWidth, encoderOutputHeight, scalerOutputWidth,
            scalerOutputHeight, cropOriginX, cropOriginY};
}
