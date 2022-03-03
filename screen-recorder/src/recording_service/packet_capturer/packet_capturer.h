#ifndef PDS_SCREEN_RECORDING_PACKET_CAPTURER_H
#define PDS_SCREEN_RECORDING_PACKET_CAPTURER_H

#include <functional>

#include "../device_context.h"
#include "../ffmpeg_objects_deleter.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

typedef std::function<void(std::unique_ptr<AVPacket, FFMpegObjectsDeleter> packet, int64_t relativePts)>
CapturedPacketHandler;

class PacketCapturer {
    std::shared_ptr<DeviceContext> inputDevice;

    int64_t totalPauseDuration = 0;

    int minFramePeriod; // Interval in milliseconds between two packets in the stream with the highest framerate (samplerate)

    CapturedPacketHandler onVideoPacketCapture;
    CapturedPacketHandler onAudioPacketCapture;

    void handle_captured_video_packet(AVPacket *inputVideoPacket);

    void handle_captured_audio_packet(AVPacket *inputAudioPacket);

public:
    PacketCapturer(std::shared_ptr<DeviceContext> inputDevice,
                   CapturedPacketHandler onVideoPacketCapture,
                   CapturedPacketHandler onAudioPacketCapture);

    void capture_next();

    void add_pause_duration(int64_t pauseDuration);

    [[nodiscard]] int64_t get_pause_duration() const;

    void sleep();

    ~PacketCapturer() = default;
};

#endif  // PDS_SCREEN_RECORDING_PACKET_CAPTURER_H
