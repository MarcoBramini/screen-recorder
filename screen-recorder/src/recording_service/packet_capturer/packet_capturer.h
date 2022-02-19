#ifndef PDS_SCREEN_RECORDING_PACKET_CAPTURER_H
#define PDS_SCREEN_RECORDING_PACKET_CAPTURER_H

#include <fmt/core.h>
#include <string>
#include "../device_context.h"
#include "../error.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

typedef std::function<void(AVPacket *packet, int64_t relativePts)>
        CapturedPacketHandler;

class PacketCapturer {
    DeviceContext *inputDevice;

    int64_t lastVideoPts = 0;
    int64_t lastAudioPts = 0;

    int64_t totalPauseDuration = 0;

    CapturedPacketHandler onVideoPacketCapture;
    CapturedPacketHandler onAudioPacketCapture;

    void handle_captured_video_packet(AVPacket *inputVideoPacket);

    void handle_captured_audio_packet(AVPacket *inputAudioPacket);

public:
    PacketCapturer(DeviceContext *inputDevice,
                   CapturedPacketHandler onVideoPacketCapture,
                   CapturedPacketHandler onAudioPacketCapture);

    void capture_next();

    void add_pause_duration(int64_t pauseDuration);

    int64_t get_pause_duration() const;

    ~PacketCapturer() {};
};

#endif  // PDS_SCREEN_RECORDING_PACKET_CAPTURER_H
