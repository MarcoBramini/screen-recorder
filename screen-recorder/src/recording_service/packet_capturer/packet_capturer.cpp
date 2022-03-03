#include "packet_capturer.h"
#include <fmt/core.h>
#include <thread>
#include "../error.h"

PacketCapturer::PacketCapturer(std::shared_ptr<DeviceContext> inputDevice,
                               CapturedPacketHandler onVideoPacketCapture,
                               CapturedPacketHandler onAudioPacketCapture)
    : inputDevice(std::move(inputDevice)),
      onVideoPacketCapture(std::move(onVideoPacketCapture)),
      onAudioPacketCapture(std::move(onAudioPacketCapture)),
      totalPauseDuration(0) {
  int videoFramePeriod = std::numeric_limits<int>::max(),
      audioFramePeriod = std::numeric_limits<int>::max();
  AVStream* videoStream = this->inputDevice->getVideoStream();
  AVStream* audioStream = this->inputDevice->getAudioStream();

  if (videoStream)
    videoFramePeriod =
        (int)(videoStream->time_base.den / av_q2d(videoStream->r_frame_rate));
  if (audioStream)
    audioFramePeriod =
        (int)(audioStream->time_base.den / audioStream->codecpar->sample_rate);

  minFramePeriod = std::min(videoFramePeriod, audioFramePeriod);
}

/// Calculates the normalized PTS of a packet.
int64_t calculate_packet_pts(int64_t absolutePts,
                             int64_t streamStartTime,
                             int64_t totalPauseDuration) {
  return absolutePts - streamStartTime - totalPauseDuration;
}

/// Handles a newly captured video packet.
/// Calculates its normalized PTS and calls the handling callback.
void PacketCapturer::handle_captured_video_packet(AVPacket* inputVideoPacket) {
  auto videoPacket = av_packet_clone(inputVideoPacket);

  int64_t packetPts = calculate_packet_pts(
      inputVideoPacket->pts, inputDevice->getVideoStream()->start_time,
      totalPauseDuration);
  // onVideoPacketCapture(videoPacket, packetPts);
}

/// Handles a newly captured audio packet.
/// Calculates its normalized PTS and calls the handling callback.
void PacketCapturer::handle_captured_audio_packet(AVPacket* inputAudioPacket) {
  auto audioPacket = av_packet_clone(inputAudioPacket);

  int64_t packetPts = calculate_packet_pts(
      inputAudioPacket->pts, inputDevice->getAudioStream()->start_time,
      totalPauseDuration);
  // onAudioPacketCapture(audioPacket, packetPts);
}

// Captures a new packet from the input device
void PacketCapturer::capture_next() {
  auto inputPacket =
      std::unique_ptr<AVPacket, FFMpegObjectsDeleter>(av_packet_alloc());

  int ret;
  do {
    ret = av_read_frame(inputDevice->getContext(), inputPacket.get());
  } while (ret == AVERROR(EAGAIN));

  if (ret < 0) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {},
        fmt::format("error reading frames from the input device ({})",
                    Error::unpackAVError(ret))));
  }

  AVMediaType packetType = inputDevice->getContext()
                               ->streams[inputPacket->stream_index]
                               ->codecpar->codec_type;

  int64_t packetPts;
  switch (packetType) {
    case AVMEDIA_TYPE_VIDEO:
      // auto videoPacket = av_packet_clone(inputVideoPacket);
      packetPts = calculate_packet_pts(
          inputPacket->pts, inputDevice->getVideoStream()->start_time,
          totalPauseDuration);

      onVideoPacketCapture(std::move(inputPacket), packetPts);
      break;
    case AVMEDIA_TYPE_AUDIO:
      packetPts = calculate_packet_pts(
          inputPacket->pts, inputDevice->getAudioStream()->start_time,
          totalPauseDuration);

      onAudioPacketCapture(std::move(inputPacket), packetPts);
      break;
    default:
      throw std::runtime_error(
          Error::build_error_message(__FUNCTION__, {},
                                     fmt::format("unexpected packet type ({})",
                                                 std::to_string(packetType))));
  }
}

// Adds the duration of a resumed pause in order to normalize the packets PTS.
void PacketCapturer::add_pause_duration(int64_t pauseDuration) {
  totalPauseDuration += pauseDuration;
}

int64_t PacketCapturer::get_pause_duration() const {
  return totalPauseDuration;
}

/// Sleeps until the next packet to capture will be available.
/// The sleep duration is obtained by the input stream framerate.
void PacketCapturer::sleep() {
  std::this_thread::sleep_for(std::chrono::microseconds(minFramePeriod));
}
