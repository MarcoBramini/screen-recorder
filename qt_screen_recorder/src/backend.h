#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include <device_service.h>
#include <recording_service.h>

class BackEnd : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString outputDir READ getOutputDir WRITE setOutputDir NOTIFY outputDirChanged)
    Q_PROPERTY(QList<QString> videoDevices READ getVideoDevices CONSTANT)
    Q_PROPERTY(QList<QString> audioDevices READ getAudioDevices CONSTANT)
    Q_PROPERTY(int selectedVideoDeviceIndex READ getSelectedVideoDeviceIndex WRITE setSelectedVideoDeviceIndex NOTIFY selectedVideoDeviceIndexChanged)
    Q_PROPERTY(int selectedAudioDeviceIndex READ getSelectedAudioDeviceIndex WRITE setSelectedAudioDeviceIndex NOTIFY selectedAudioDeviceIndexChanged)
    Q_PROPERTY(QList<QString> outputResolutions READ getOutputResolutions CONSTANT)
    Q_PROPERTY(int selectedOutputResolutionIndex READ getSelectedOutputResolutionIndex WRITE setSelectedOutputResolutionIndex NOTIFY selectedOutputResolutionIndexChanged)
    Q_PROPERTY(QVariantMap selectedCaptureRegion READ getSelectedCaptureRegion WRITE setSelectedCaptureRegion NOTIFY selectedCaptureRegionChanged)
    Q_PROPERTY(QList<QString> framerates READ getFramerates CONSTANT)
    Q_PROPERTY(int selectedFramerateIndex READ getSelectedFramerateIndex WRITE setSelectedFramerateIndex NOTIFY selectedFramerateIndexChanged)
    Q_PROPERTY(QString errorMessage READ getErrorMessage WRITE setErrorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(QVariantMap permissionsStatus READ getPermissionsStatus NOTIFY permissionsStatusChanged)

    QML_ELEMENT

    QString m_outputDir;

    std::vector<InputDeviceVideo> availableVideoDevices;
    std::vector<InputDeviceAudio> availableAudioDevices;
    int m_selectedVideoDeviceIndex;
    int m_selectedAudioDeviceIndex;

    std::vector<std::tuple<int,int,double>> availableOutputResolutions;
    int m_selectedOutputResolutionIndex;

    QVariantMap m_selectedCaptureRegion;

    std::vector<int> availableFramerates;
    int m_selectedFramerateIndex;

    RecordingConfig config;
    std::unique_ptr<RecordingService> rs;

    QString m_errorMessage;

    PermissionsStatus permissionsStatus;

    void updateAvailableOutputResolutions();
public:
    explicit BackEnd(QObject *parent = nullptr);

    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void pauseRecording();
    Q_INVOKABLE void resumeRecording();
    Q_INVOKABLE QVariantMap getRecordingStats();

    Q_INVOKABLE void restartApp();

    // ------
    // Config
    // ------

    // Output path
    QString getOutputDir();

    void setOutputDir(QString path);

    // A/V input devices
    QList<QString> getVideoDevices();
    QList<QString> getAudioDevices();

    int getSelectedVideoDeviceIndex();
    int getSelectedAudioDeviceIndex();

    void setSelectedVideoDeviceIndex(int index);
    void setSelectedAudioDeviceIndex(int index);

    // Output resolution
    QList<QString> getOutputResolutions();

    int getSelectedOutputResolutionIndex();

    void setSelectedOutputResolutionIndex(int index);

    // Capture region
    QVariantMap getSelectedCaptureRegion();

    void setSelectedCaptureRegion(QVariantMap captureRegion);

    Q_INVOKABLE void resetCaptureRegion();

    // Framerate
    QList<QString> getFramerates();

    int getSelectedFramerateIndex();

    void setSelectedFramerateIndex(int index);

    // -----
    // Error
    // -----
    QString getErrorMessage();

    void setErrorMessage(QString message);

    // ----------------
    // PermissionStatus
    // ----------------
    Q_INVOKABLE bool showPermissionsSetup();

    QVariantMap getPermissionsStatus();

    void setCameraCapturePermissionStatus(bool isGranted);

    void setMicrophoneCapturePermissionStatus(bool isGranted);

    void setScreenCapturePermissionStatus(bool isGranted);

    Q_INVOKABLE void setupCameraUsagePermission();
    Q_INVOKABLE void setupMicrophoneUsagePermission();
    Q_INVOKABLE void setupScreenCapturePermission();

signals:
    void outputDirChanged();
    void selectedVideoDeviceIndexChanged();
    void selectedAudioDeviceIndexChanged();
    void selectedOutputResolutionIndexChanged();
    void selectedCaptureRegionChanged();
    void selectedFramerateIndexChanged();
    void errorMessageChanged();
    void permissionsStatusChanged();
};

#endif // BACKEND_H
