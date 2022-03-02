#include "backend.h"
#include <QDateTime>
#include <QGuiApplication>
#include <QProcess>
#include <QScreen>
#include <QStandardPaths>
#include <stdexcept>

std::tuple<int, int> getScreenResolution() {
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    QRect screenGeometry = primaryScreen->geometry();
    return {screenGeometry.width(), screenGeometry.height()};
}

double getDevicePixelRatio() {
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    double devicePixelRatio = primaryScreen->devicePixelRatio();
    return devicePixelRatio;
}

BackEnd::BackEnd(QObject* parent) : QObject(parent) {
    m_errorMessage = "";
    m_outputDir = "";
    m_selectedAudioDeviceIndex = -1;
    m_selectedCaptureRegion = {};
    m_selectedFramerateIndex = -1;
    m_selectedOutputResolutionIndex = -1;
    m_selectedVideoDeviceIndex = -1;

    permissionsStatus = DeviceService::check_permissions();

    availableVideoDevices = DeviceService::get_input_video_devices();
    availableAudioDevices = DeviceService::get_input_audio_devices();

    auto [width, height] = getScreenResolution();
    double devicePixelRatio = getDevicePixelRatio();
    availableOutputResolutions = RecordingConfig::getOutputResolutionsChoices(
                                     width * devicePixelRatio, height * devicePixelRatio);

    config = {};
    config.setUseControlThread(false);

    // Init output path
    QStringList locations =
        QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    setOutputDir(locations[0]);

    // Init devices
    setSelectedVideoDeviceIndex(0);
    setSelectedAudioDeviceIndex(0);

    // Init framerate
    availableFramerates = {20, 24, 30, 60};
    setSelectedFramerateIndex(2);

    // Init output resolution
    setSelectedOutputResolutionIndex(0);
}

void BackEnd::startRecording() {
    try {
        std::cout << config.getVideoAddress() << std::endl;
        rs = std::move(std::make_unique<RecordingService>(config));
        rs->start_recording();
    } catch (std::runtime_error error) {
        setErrorMessage(QString{error.what()});
        emit errorMessageChanged();
    }
}

void BackEnd::stopRecording() {
    try {
        rs->stop_recording();
        rs.reset();
    } catch (std::runtime_error error) {
        setErrorMessage(QString{error.what()});
        emit errorMessageChanged();
    }
}

void BackEnd::pauseRecording() {
    try {
        rs->pause_recording();
    } catch (std::runtime_error error) {
        setErrorMessage(QString{error.what()});
        emit errorMessageChanged();
    }
}

void BackEnd::resumeRecording() {
    try {
        rs->resume_recording();
    } catch (std::runtime_error error) {
        setErrorMessage(QString{error.what()});
        emit errorMessageChanged();
    }
}

QVariantMap BackEnd::getRecordingStats() {
    if (rs == nullptr)
        return {};

    QVariantMap output;
    RecordingStats stats = rs->get_recording_stats();
    output["status"] = stats.status;

    QDateTime timestamp;
    timestamp.setSecsSinceEpoch(stats.recordingDuration);
    output["recordingDuration"] = timestamp.toUTC().toString("HH:mm:ss");

    return output;
}

void BackEnd::restartApp() {
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

QString BackEnd::getOutputDir() {
    return m_outputDir;
}

void BackEnd::setOutputDir(QString dir) {
    if (dir == m_outputDir)
        return;

    config.setOutputDir(dir.toStdString());
    m_outputDir = dir;
    emit outputDirChanged();
}

QList<QString> BackEnd::getVideoDevices() {
    QList<QString> output;
    for (auto device : availableVideoDevices) {
        output.emplaceBack(device.getName().c_str());
    }

    return output;
}

QList<QString> BackEnd::getAudioDevices() {
    QList<QString> output;
    for (auto device : availableAudioDevices) {
        output.emplaceBack(device.getName().c_str());
    }

    // Allow users to disable the audio recording
    output.emplaceBack("Disable audio");

    return output;
}

QList<QString> BackEnd::getOutputResolutions() {
    QList<QString> output;
    for (auto r : availableOutputResolutions) {
        auto [width, height, factor] = r;
        output.emplaceBack(
            (std::to_string(width) + "x" + std::to_string(height)).c_str());
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
    if (index == m_selectedVideoDeviceIndex)
        return;

    m_selectedVideoDeviceIndex = index;
    config.setVideoAddress(availableVideoDevices[index].getDeviceAddress());
    emit selectedVideoDeviceIndexChanged();
}

void BackEnd::setSelectedAudioDeviceIndex(int index) {
    if (index == m_selectedAudioDeviceIndex)
        return;

    m_selectedAudioDeviceIndex = index;
    if (index >= availableAudioDevices.size()) {
        config.disableAudio();
    } else {
        config.setAudioAddress(availableAudioDevices[index].getDeviceAddress());
    }
    emit selectedAudioDeviceIndexChanged();
}

int BackEnd::getSelectedOutputResolutionIndex() {
    return m_selectedOutputResolutionIndex;
}

void BackEnd::setSelectedOutputResolutionIndex(int index) {
    if (index == m_selectedOutputResolutionIndex)
        return;

    m_selectedOutputResolutionIndex = index;
    config.setOutputResolution(availableOutputResolutions[index]);
    emit selectedOutputResolutionIndexChanged();
}

QVariantMap BackEnd::getSelectedCaptureRegion() {
    return m_selectedCaptureRegion;
}

// Updates the available resolutions and resets the user choice to default
void BackEnd::updateAvailableOutputResolutions() {
    int width, height;
    double devicePixelRatio = getDevicePixelRatio();
    if (config.getCaptureRegion().has_value()) {
        std::tie(std::ignore, std::ignore, width, height) =
            config.getCaptureRegion().value();
    } else {
        std::tie(width, height) = getScreenResolution();
        width *= devicePixelRatio;
        height *= devicePixelRatio;
    }
    availableOutputResolutions =
        RecordingConfig::getOutputResolutionsChoices(width, height);
    setSelectedOutputResolutionIndex(0);
}

void BackEnd::setSelectedCaptureRegion(QVariantMap captureRegion) {
    double devicePixelRatio = getDevicePixelRatio();

    captureRegion["x"] = captureRegion["x"].toInt() * devicePixelRatio;
    captureRegion["y"] = captureRegion["y"].toInt() * devicePixelRatio;
    captureRegion["width"] = captureRegion["width"].toInt() * devicePixelRatio;
    captureRegion["height"] = captureRegion["height"].toInt() * devicePixelRatio;

    m_selectedCaptureRegion = captureRegion;
    config.setCaptureRegion(
        captureRegion["x"].toInt(), captureRegion["y"].toInt(),
        captureRegion["width"].toInt(), captureRegion["height"].toInt());

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
    for (auto f : availableFramerates) {
        output.emplaceBack(std::to_string(f).c_str());
    }

    return output;
}

int BackEnd::getSelectedFramerateIndex() {
    return m_selectedFramerateIndex;
}

void BackEnd::setSelectedFramerateIndex(int index) {
    if (index == m_selectedFramerateIndex)
        return;

    m_selectedFramerateIndex = index;

    config.setFramerate(availableFramerates[index]);

    emit selectedFramerateIndexChanged();
}

QString BackEnd::getErrorMessage() {
    return m_errorMessage;
}

void BackEnd::setErrorMessage(QString message) {
    if (message == m_errorMessage)
        return;

    m_errorMessage = message;
    emit errorMessageChanged();
}

bool BackEnd::showPermissionsSetup() {
    return !(permissionsStatus.cameraCaptureAllowed &&
             permissionsStatus.microphoneCaptureAllowed &&
             permissionsStatus.screenCaptureAllowed);
}

QVariantMap BackEnd::getPermissionsStatus() {
    QVariantMap output;
    output["screenCaptureAllowed"] = permissionsStatus.screenCaptureAllowed;
    output["microphoneCaptureAllowed"] =
        permissionsStatus.microphoneCaptureAllowed;
    output["cameraCaptureAllowed"] = permissionsStatus.cameraCaptureAllowed;
    return output;
}

void BackEnd::setCameraCapturePermissionStatus(bool isGranted) {
    permissionsStatus.cameraCaptureAllowed = isGranted;

    emit permissionsStatusChanged();
}

void BackEnd::setMicrophoneCapturePermissionStatus(bool isGranted) {
    permissionsStatus.microphoneCaptureAllowed = isGranted;

    emit permissionsStatusChanged();
}

void BackEnd::setScreenCapturePermissionStatus(bool isGranted) {
    permissionsStatus.screenCaptureAllowed = isGranted;

    emit permissionsStatusChanged();
}

void BackEnd::setupCameraUsagePermission() {
    DeviceService::setup_camera_usage_permission([&](bool isGranted) {
        this->setCameraCapturePermissionStatus(isGranted);
    });
}

void BackEnd::setupMicrophoneUsagePermission() {
    DeviceService::setup_microphone_usage_permission([&](bool isGranted) {
        this->setMicrophoneCapturePermissionStatus(isGranted);
    });
}

void BackEnd::setupScreenCapturePermission() {
    DeviceService::setup_screen_capture_permission([&](bool isGranted) {
        this->setScreenCapturePermissionStatus(isGranted);
    });
}
