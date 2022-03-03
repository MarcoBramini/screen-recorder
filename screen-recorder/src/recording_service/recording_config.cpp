#include "recording_config.h"
#include <fmt/core.h>
#include <filesystem>
#include <chrono>

const std::string &RecordingConfig::getVideoAddress() const {
    return videoAddress;
}

/// Sets the input video device to use for recording.
/// Refer to the class documentation for information about the allowed formats.
void RecordingConfig::setVideoAddress(const std::string &address) {
    videoAddress = address;
}

const std::string &RecordingConfig::getAudioAddress() const {
    return audioAddress;
}

/// Sets the input audio device to use for recording.
/// Refer to the class documentation for information about the allowed formats.
void RecordingConfig::setAudioAddress(const std::string &address) {
    audioAddress = address;
}

/// Disables the audio capture
void RecordingConfig::disableAudio() {
    audioAddress = "";
}

const std::string &RecordingConfig::getOutputDir() const {
    return outputDir;
}

/// Returns the full path of the recording output file.
std::string RecordingConfig::getOutputPath() const {
    std::filesystem::path dir(outputDir);

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::filesystem::path filename(
            fmt::format("rec_{}-{}-{}T{}-{}.mp4", tm.tm_year, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min));

    std::string output = (dir / filename).string();

    return output;
}

/// Sets the output folder in which the output file will be saved.
/// Refer to the class documentation for information about the allowed formats.
void RecordingConfig::setOutputDir(const std::string &filename) {
    outputDir = filename;
}

const std::optional<std::tuple<int, int, int, int>> &RecordingConfig::getCaptureRegion() const {
    return captureRegion;
}

/// Sets the capture region to use for recording.
/// Refer to the class documentation for information about the allowed formats.
void RecordingConfig::setCaptureRegion(int x, int y, int width, int height) {
    captureRegion = {x, y, width, height};
}

void RecordingConfig::resetCaptureRegion() {
    captureRegion.reset();
}

int RecordingConfig::getFramerate() const {
    return framerate;
}

/// Sets the output framerate.
/// Refer to the class documentation for information about the allowed formats.
void RecordingConfig::setFramerate(int value) {
    framerate = value;
}

inline int make_even(int n) {
    return n - n % 2;
}

/// Returns a list of resolutions which the user can select.
/// The available resolutions are obtained by scaling the input resolution by these factors: [1, 0.75, 0.5, 0.25]
std::vector<std::tuple<int, int, double>>
RecordingConfig::getOutputResolutionsChoices(int inputWidth, int inputHeight) {
    std::vector<std::tuple<int, int, double>> outputResolutions;

    std::vector<double> factors = {1, 0.75, 0.5, 0.25};

    outputResolutions.reserve(factors.size());
    for (auto factor: factors) {
        outputResolutions.emplace_back(make_even((int) (inputWidth * factor)), make_even((int) (inputHeight * factor)),
                                       factor);
    }
    return outputResolutions;
}

const std::optional<std::tuple<int, int, double>> &RecordingConfig::getOutputResolution() const {
    return outputResolution;
}

/// Sets the output resolution.
/// Refer to the class documentation for information about the allowed formats.
void RecordingConfig::setOutputResolution(std::tuple<int, int, double> resolution) {
    outputResolution = resolution;
}

bool RecordingConfig::isUseControlThread() const {
    return useControlThread;
}

/// Sets if the internal control thread must be used.
/// Refer to the class documentation for information about the allowed formats.
void RecordingConfig::setUseControlThread(bool enabled) {
    useControlThread = enabled;
}


