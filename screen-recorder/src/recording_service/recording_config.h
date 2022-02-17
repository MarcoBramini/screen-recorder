#ifndef PDS_SCREEN_RECORDING_RECORDINGCONFIG_H
#define PDS_SCREEN_RECORDING_RECORDINGCONFIG_H

#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <string>
#include <thread>

class RecordingConfig {
    std::string videoAddress;
    std::string audioAddress;
    std::string outputFilename;
    std::optional<std::tuple<int, int, int, int>> captureRegion;            // x,y,width,height from top left
    std::optional<std::tuple<int, int>> outputResolution;                // width,height
    int framerate = 30;

public:
    [[nodiscard]] const std::string &getVideoAddress() const;

    void setVideoAddress(const std::string &address);

    [[nodiscard]] const std::string &getAudioAddress() const;

    void setAudioAddress(const std::string &address);

    [[nodiscard]] const std::string &getOutputFilename() const;

    void setOutputFilename(const std::string &filename);

    [[nodiscard]] const std::optional<std::tuple<int, int, int, int>> &getCaptureRegion() const;

    void setCaptureRegion(int x, int y, int width, int height);

    void resetCaptureRegion();

    static std::vector<std::tuple<int, int>> getOutputResolutionsChoices(int inputWidth, int inputHeight);

    [[nodiscard]] const std::optional<std::tuple<int, int>> &getOutputResolution() const;

    void setOutputResolution(std::tuple<int, int> resolution);

    [[nodiscard]] int getFramerate() const;

    void setFramerate(int framerate);
};

#endif  // PDS_SCREEN_RECORDING_RECORDINGCONFIG_H
