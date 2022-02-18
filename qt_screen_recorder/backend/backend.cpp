#include "backend.h"
#include <QStandardPaths>
#include <QGuiApplication>
#include <QScreen>

std::tuple<int,int> getScreenResolution() {
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    QRect screenGeometry = primaryScreen->geometry();
    return {screenGeometry.width(), screenGeometry.height()};
}

BackEnd::BackEnd(QObject *parent) :
    QObject(parent)
{
    availableVideoDevices = DeviceService::get_input_video_devices();
    availableAudioDevices = DeviceService::get_input_audio_devices();

    auto [width, height] = getScreenResolution();
    availableOutputResolutions = RecordingConfig::getOutputResolutionsChoices(width, height);

    config = {};
    config.setUseControlThread(false);

    // Init output path
    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    setOutputDir(locations[0]);

    // Init devices
    setSelectedVideoDeviceIndex(0);
    setSelectedAudioDeviceIndex(0);

    // Init framerate
    availableFramerates = {20,24,30,60};
    setSelectedFramerateIndex(2);

    // Init output resolution
    setSelectedOutputResolutionIndex(0);
}

void BackEnd::startRecording() {
    rs = std::move(std::make_unique<RecordingService>(config));
    rs->start_recording();
}

void BackEnd::stopRecording() {
    rs->stop_recording();
}

void BackEnd::pauseRecording() {
    rs->pause_recording();
}

void BackEnd::resumeRecording() {
    rs->resume_recording();
}

QString BackEnd::getOutputDir() {
    return m_outputDir;
}

void BackEnd::setOutputDir(QString dir) {
    if (dir == m_outputDir) return;

    config.setOutputDir(dir.toStdString());
    m_outputDir = dir;
    emit outputDirChanged();
}

QList<QString> BackEnd::getVideoDevices()
{
    QList<QString> output;
    for (auto device:availableVideoDevices) {
        output.emplaceBack(device.getName().c_str());
    }

    return output;
}

QList<QString> BackEnd::getAudioDevices()
{
    QList<QString> output;
    for (auto device:availableAudioDevices) {
        output.emplaceBack(device.getName().c_str());
    }

    return output;
}

QList<QString> BackEnd::getOutputResolutions() {
    QList<QString> output;
    for (auto r:availableOutputResolutions) {
        auto [width, height] = r;
        output.emplaceBack((std::to_string(width)+"x"+std::to_string(height)).c_str());
    }

    return output;
}

int BackEnd::getSelectedVideoDeviceIndex() {
    return m_selectedVideoDeviceIndex;
}

int BackEnd::getSelectedAudioDeviceIndex() {
    return m_selectedAudioDeviceIndex;
}

void BackEnd::setSelectedVideoDeviceIndex(int index) {
    if (index == m_selectedVideoDeviceIndex) return;

    m_selectedVideoDeviceIndex = index;
    config.setVideoAddress(availableVideoDevices[index].getDeviceAddress());
    emit selectedVideoDeviceIndexChanged();
}

void BackEnd::setSelectedAudioDeviceIndex(int index) {
    if (index == m_selectedAudioDeviceIndex) return;

    m_selectedAudioDeviceIndex = index;
    config.setAudioAddress(availableAudioDevices[index].getDeviceAddress());
    emit selectedAudioDeviceIndexChanged();
}

int BackEnd::getSelectedOutputResolutionIndex() {
    return m_selectedOutputResolutionIndex;
}

void BackEnd::setSelectedOutputResolutionIndex(int index) {
    std::cout<<"asd"<<std::endl;
    if (index == m_selectedOutputResolutionIndex) return;

    m_selectedOutputResolutionIndex = index;
    config.setOutputResolution(availableOutputResolutions[index]);
    emit selectedOutputResolutionIndexChanged();
}

QVariantMap BackEnd::getSelectedCaptureRegion() {
    return m_selectedCaptureRegion;
}

// Updates the available resolutions and resets the user choice to default
void BackEnd::updateAvailableOutputResolutions() {
    int width,height;
    if (config.getCaptureRegion().has_value()) {
        std::tie(std::ignore,std::ignore,width, height) = config.getCaptureRegion().value();
    } else {
        std::tie(width, height) = getScreenResolution();
    }
    availableOutputResolutions = RecordingConfig::getOutputResolutionsChoices(width, height);
    setSelectedOutputResolutionIndex(0);
}

void BackEnd::setSelectedCaptureRegion(QVariantMap captureRegion) {
    if (captureRegion == m_selectedCaptureRegion) return;

    m_selectedCaptureRegion = captureRegion;
    config.setCaptureRegion(captureRegion["x"].toInt(),captureRegion["y"].toInt(),captureRegion["width"].toInt(), captureRegion["height"].toInt());

    updateAvailableOutputResolutions();

    emit selectedCaptureRegionChanged();
}

void BackEnd::resetCaptureRegion() {
    m_selectedCaptureRegion = {};
    config.resetCaptureRegion();

    updateAvailableOutputResolutions();

    emit selectedCaptureRegionChanged();
}

QList<QString> BackEnd::getFramerates() {
    QList<QString> output;
    for (auto f:availableFramerates) {
        output.emplaceBack(std::to_string(f).c_str());
    }

    return output;
}

int BackEnd::getSelectedFramerateIndex() {
    return m_selectedFramerateIndex;
}

void BackEnd::setSelectedFramerateIndex(int index) {
    if (index == m_selectedFramerateIndex) return;

    m_selectedFramerateIndex = index;

    config.setFramerate(availableFramerates[index]);

    emit selectedFramerateIndexChanged();
}
