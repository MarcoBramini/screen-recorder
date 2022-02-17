#include "recording_config.h"
#include <fmt/core.h>

const std::string &RecordingConfig::getVideoAddress() const {
    return videoAddress;
}

void RecordingConfig::setVideoAddress(const std::string &address) {
    videoAddress = address;
}

const std::string &RecordingConfig::getAudioAddress() const {
    return audioAddress;
}

void RecordingConfig::setAudioAddress(const std::string &address) {
    audioAddress = address;
}

const std::string &RecordingConfig::getOutputFilename() const {
    return outputFilename;
}

void RecordingConfig::setOutputFilename(const std::string &filename) {
    outputFilename = filename;
}

const std::optional<std::tuple<int, int, int, int>> &RecordingConfig::getCaptureRegion() const {
    return captureRegion;
}

void RecordingConfig::setCaptureRegion(int x, int y, int width, int height) {
    captureRegion = {x, y, width, height};
}

void RecordingConfig::resetCaptureRegion() {
    captureRegion.reset();
}

int RecordingConfig::getFramerate() const {
    return framerate;
}

void RecordingConfig::setFramerate(int value) {
    framerate = value;
}

inline int make_even(int n) {
    return n - n % 2;
}

// Returns a list of resolutions which the user can select.
// The resolutions are obtained by scaling the input resolution by these factors: [1, 0.75, 0.5, 0.25]
std::vector<std::tuple<int, int>> RecordingConfig::getOutputResolutionsChoices(int inputWidth, int inputHeight) {
    std::vector<std::tuple<int, int>> outputResolutions;

    std::vector<double> factors = {1, 0.75, 0.5, 0.25};

    outputResolutions.reserve(factors.size());
    for (auto factor: factors) {
        outputResolutions.emplace_back(make_even((int) (inputWidth * factor)), make_even((int) (inputHeight * factor)));
    }
    return outputResolutions;
}

const std::optional<std::tuple<int, int>> &RecordingConfig::getOutputResolution() const {
    return outputResolution;
}

void RecordingConfig::setOutputResolution(std::tuple<int, int> resolution) {
    RecordingConfig::outputResolution = resolution;
}


