#include "backend.h"
#include <QStandardPaths>
#include <QGuiApplication>
#include <QScreen>

BackEnd::BackEnd(QObject *parent) :
    QObject(parent)
{
    availableVideoDevices = DeviceService::get_input_video_devices();
    availableAudioDevices = DeviceService::get_input_audio_devices();

    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    QRect screenGeometry = primaryScreen->geometry();
    availableOutputResolutions = RecordingConfig::getOutputResolutionsChoices(screenGeometry.width(), screenGeometry.height());

    config = {};

    // Init output patj
    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    setOutputPath(locations[0]);

    // Init devices
    setSelectedVideoDeviceIndex(0);
    setSelectedAudioDeviceIndex(0);

    // Init framerate

    // Init capture region

    // Init output resolution
    setSelectedOutputResolutionIndex(0);


}

QString BackEnd::getOutputPath() {
    return m_outputPath;
}

void BackEnd::setOutputPath(QString path) {
    if (path == m_outputPath) return;

    config.setOutputFilename(path.toStdString());
    m_outputPath = path;
    emit outputPathChanged();
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
    if (index == m_selectedOutputResolutionIndex) return;

    m_selectedOutputResolutionIndex = index;
    config.setOutputResolution(availableOutputResolutions[index]);
    emit selectedOutputResolutionIndexChanged();
}

QVariantMap BackEnd::getSelectedCaptureRegion() {
    return m_selectedCaptureRegion;
}

void BackEnd::setSelectedCaptureRegion(QVariantMap captureRegion) {
    if (captureRegion == m_selectedCaptureRegion) return;

    m_selectedCaptureRegion = captureRegion;
    config.setCaptureRegion(captureRegion["x"].toInt(),captureRegion["y"].toInt(),captureRegion["width"].toInt(), captureRegion["height"].toInt());

    // Update output resolution
    auto [x,y,width, height] = config.getCaptureRegion().value();
    availableOutputResolutions = RecordingConfig::getOutputResolutionsChoices(width, height);
    setSelectedOutputResolutionIndex(2);

    emit selectedOutputResolutionIndexChanged();
}
