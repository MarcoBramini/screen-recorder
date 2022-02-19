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
    std::string outputDir;
    std::string outputPath;
    std::optional<std::tuple<int, int, int, int>> captureRegion;            // x,y,width,height from top left
    std::optional<std::tuple<int, int, double>> outputResolution;                // width,height,scalingFactor
    int framerate = 30;
    bool useControlThread = true;     // used to keep the process running without assigning a thread

public:
    [[nodiscard]] const std::string &getVideoAddress() const;

    void setVideoAddress(const std::string &address);

    [[nodiscard]] const std::string &getAudioAddress() const;

    void setAudioAddress(const std::string &address);

    void disableAudio();

    [[nodiscard]] const std::string &getOutputDir() const;

    void setOutputDir(const std::string &filename);

    [[nodiscard]] std::string getOutputPath() const;

    [[nodiscard]] const std::optional<std::tuple<int, int, int, int>> &getCaptureRegion() const;

    void setCaptureRegion(int x, int y, int width, int height);

    void resetCaptureRegion();

    static std::vector<std::tuple<int, int, double>> getOutputResolutionsChoices(int inputWidth, int inputHeight);

    [[nodiscard]] const std::optional<std::tuple<int, int, double>> &getOutputResolution() const;

    void setOutputResolution(std::tuple<int, int, double> resolution);

    [[nodiscard]] int getFramerate() const;

    void setFramerate(int framerate);

    [[nodiscard]] bool isUseControlThread() const;

    void setUseControlThread(bool enabled);
};

#endif  // PDS_SCREEN_RECORDING_RECORDINGCONFIG_H
