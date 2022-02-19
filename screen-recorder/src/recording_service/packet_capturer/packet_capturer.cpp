#include "packet_capturer.h"

PacketCapturer::PacketCapturer(DeviceContext *inputDevice,
                               CapturedPacketHandler onVideoPacketCapture,
                               CapturedPacketHandler onAudioPacketCapture)
        : inputDevice(inputDevice),
          onVideoPacketCapture(onVideoPacketCapture),
          onAudioPacketCapture(onAudioPacketCapture) {}

// Calculates the normalized PTS of a packet.
int64_t calculate_packet_pts(int64_t absolutePts,
                             int64_t streamStartTime,
                             int64_t totalPauseDuration) {
    return absolutePts - streamStartTime - totalPauseDuration;
}

// Handles a newly captured video packet.
// Calculates its normalized PTS and calls the handling callback.
void PacketCapturer::handle_captured_video_packet(AVPacket *inputVideoPacket) {
    auto videoPacket = av_packet_clone(inputVideoPacket);

    int64_t packetPts = calculate_packet_pts(
            inputVideoPacket->pts, inputDevice->getVideoStream()->start_time,
            totalPauseDuration);
    onVideoPacketCapture(videoPacket, packetPts);

    lastVideoPts = packetPts;
}

// Handles a newly captured audio packet.
// Calculates its normalized PTS and calls the handling callback.
void PacketCapturer::handle_captured_audio_packet(AVPacket *inputAudioPacket) {
    auto audioPacket = av_packet_clone(inputAudioPacket);

    int64_t packetPts = calculate_packet_pts(
            inputAudioPacket->pts, inputDevice->getAudioStream()->start_time,
            totalPauseDuration);
    onAudioPacketCapture(audioPacket, packetPts);

    lastAudioPts = packetPts;
}

// Captures a new packet from the input device
void PacketCapturer::capture_next() {
    AVPacket inputPacket;
    int ret = 0;

    do {
        ret = av_read_frame(inputDevice->getContext(), &inputPacket);
    } while (ret == AVERROR(EAGAIN));

    if (ret < 0) {
        std::string error = "Capture failed with:";
        error.append(Error::unpackAVError(ret));
        throw std::runtime_error(error);
    }

    AVMediaType packetType = inputDevice->getContext()
            ->streams[inputPacket.stream_index]
            ->codecpar->codec_type;

    switch (packetType) {
        case AVMEDIA_TYPE_VIDEO:
            PacketCapturer::handle_captured_video_packet(&inputPacket);
            break;
        case AVMEDIA_TYPE_AUDIO:
            PacketCapturer::handle_captured_audio_packet(&inputPacket);
            break;
        default:
            throw std::runtime_error(Error::build_error_message(
                    __FUNCTION__, {}, fmt::format("unexpected packet type ({})", ret)));
    }

    av_packet_unref(&inputPacket);
}

// Adds the duration of a resumed pause in order to normalize the packets PTS.
void PacketCapturer::add_pause_duration(int64_t pauseDuration) {
    totalPauseDuration += pauseDuration;
}

int64_t PacketCapturer::get_pause_duration() const {
    return totalPauseDuration;
}
