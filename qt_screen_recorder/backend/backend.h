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
    Q_PROPERTY(QString outputPath READ getOutputPath WRITE setOutputPath NOTIFY outputPathChanged)
    Q_PROPERTY(QList<QString> videoDevices READ getVideoDevices CONSTANT)
    Q_PROPERTY(QList<QString> audioDevices READ getAudioDevices CONSTANT)
    Q_PROPERTY(int selectedVideoDeviceIndex READ getSelectedVideoDeviceIndex WRITE setSelectedVideoDeviceIndex NOTIFY selectedVideoDeviceIndexChanged)
    Q_PROPERTY(int selectedAudioDeviceIndex READ getSelectedAudioDeviceIndex WRITE setSelectedAudioDeviceIndex NOTIFY selectedAudioDeviceIndexChanged)
    Q_PROPERTY(QList<QString> outputResolutions READ getOutputResolutions CONSTANT)
    Q_PROPERTY(int selectedOutputResolutionIndex READ getSelectedOutputResolutionIndex WRITE setSelectedOutputResolutionIndex NOTIFY selectedOutputResolutionIndexChanged)
    Q_PROPERTY(QVariantMap selectedCaptureRegion READ getSelectedCaptureRegion WRITE setSelectedCaptureRegion NOTIFY selectedCaptureRegionChanged)

    QML_ELEMENT

    QString m_outputPath;

    std::vector<InputDeviceVideo> availableVideoDevices;
    std::vector<InputDeviceAudio> availableAudioDevices;
    int m_selectedVideoDeviceIndex;
    int m_selectedAudioDeviceIndex;

    std::vector<std::tuple<int,int>> availableOutputResolutions;
    int m_selectedOutputResolutionIndex;

    QVariantMap m_selectedCaptureRegion;

    RecordingConfig config;

public:
    explicit BackEnd(QObject *parent = nullptr);

    // ------
    // Config
    // ------

    // Output path
    QString getOutputPath();

    void setOutputPath(QString path);

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

signals:
    void outputPathChanged();
    void selectedVideoDeviceIndexChanged();
    void selectedAudioDeviceIndexChanged();
    void selectedOutputResolutionIndexChanged();
    void selectedCaptureRegionChanged();
};

#endif // BACKEND_H
