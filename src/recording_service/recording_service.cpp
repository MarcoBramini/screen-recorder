#include "recording_service.h"

#include <fmt/core.h>

#include <chrono>
#include <csignal>
#include <map>
#include <thread>

#include "device_context.h"
#include "error.h"
#include "process_chain/decoder_ring.h"
#include "process_chain/encoder_ring.h"
#include "process_chain/muxer_ring.h"
#include "process_chain/process_chain.h"
#include "process_chain/swresample_filter_ring.h"
#include "process_chain/swscale_filter_ring.h"
#include "process_chain/vfcrop_filter_ring.h"

static bool mustTerminateSignal = false;

int64_t last_video_pts = 0;

void RecordingService::enqueue_video_packet(DeviceContext* inputDevice,
                                            AVPacket* inputVideoPacket) {
  auto videoPacket = av_packet_clone(inputVideoPacket);

  int64_t relativePts =
      inputVideoPacket->pts - inputDevice->getVideoStream()->start_time;
  videoTranscodeChain->enqueueSourcePacket(videoPacket, relativePts);
  last_video_pts = relativePts;
}

int64_t last_audio_pts = 0;

void RecordingService::enqueue_audio_packet(DeviceContext* inputDevice,
                                            AVPacket* inputAudioPacket) {
  auto audioPacket = av_packet_clone(inputAudioPacket);

  int64_t relativePts =
      inputAudioPacket->pts - inputDevice->getAudioStream()->start_time;
  audioTranscodeChain->enqueueSourcePacket(audioPacket, relativePts);

  last_audio_pts = relativePts;
}

int RecordingService::start_capture_loop(DeviceContext* inputDevice) {
  AVPacket inputPacket;
  while (recordingStatus == RECORDING) {
    int ret = av_read_frame(inputDevice->getContext(), &inputPacket);
    if (ret == AVERROR(EAGAIN))
      continue;

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
        enqueue_video_packet(inputDevice, &inputPacket);
        break;
      case AVMEDIA_TYPE_AUDIO:
        enqueue_audio_packet(inputDevice, &inputPacket);
        break;
      default:
        throw std::runtime_error(Error::build_error_message(
            __FUNCTION__, {}, fmt::format("unexpected packet type ({})", ret)));
    }
    av_packet_unref(&inputPacket);
  }

  return 0;
}

int64_t last_processed_video_pts = 0;
int64_t last_processed_audio_pts = 0;

void RecordingService::start_transcode_process(ProcessChain* transcodeChain) {
  while (true) {
    if (transcodeChain->isSourceQueueEmpty()) {
      if (recordingStatus == RECORDING)
        continue;
      break;
    }

    transcodeChain->processNext();
  }
}

void RecordingService::rec_stats_loop() {
  while (recordingStatus == RECORDING) {
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    /*std::cout << "\r Packet Queue Size: " << capturedVideoPacketsQueue.size()
              << "Last Captured PTS - video: "
              << av_rescale_q(last_video_pts, outputVideoAvcc->time_base, {1,
       1000}) << " audio: "
              << av_rescale_q(last_audio_pts, outputAudioAvcc->time_base, {1,
       1000})
              << "Last Processed PTS - video: "
              << av_rescale_q(last_processed_video_pts,
       outputVideoAvcc->time_base, {1, 1000}) << " audio: "
              << av_rescale_q(last_processed_audio_pts,
       outputAudioAvcc->time_base, {1, 1000});*/
  }
}

int RecordingService::start_recording() {
  // Write output file header
  int ret = avformat_write_header(outputMuxer->getContext(), nullptr);
  if (ret < 0) {
    // Handle
    return -1;
  }

  recordingStatus = RECORDING;

  // Call start_recording_loop in a new thread
  videoCaptureThread =
      std::thread([this]() { start_capture_loop(mainDevice); });

  if (mainDevice != auxDevice) {
    audioCaptureThread =
        std::thread([this]() { start_capture_loop(auxDevice); });
  }

  capturedVideoPacketsProcessThread =
      std::thread([this]() { start_transcode_process(videoTranscodeChain); });

  capturedAudioPacketsProcessThread =
      std::thread([this]() { start_transcode_process(audioTranscodeChain); });

  recordingStatsThread = std::thread([this]() { rec_stats_loop(); });

  return 0;
}

int RecordingService::pause_recording() {
  // Set pauseTimestamp

  // Set cond var isPaused to true
  recordingStatus = PAUSE;

  return 0;
}

int RecordingService::resume_recording() {
  // Increment pausedTime by resumeTimestamp - pauseTimestamp interval

  // Set cond var isPaused to false
  recordingStatus = RECORDING;
  return 0;
}

int RecordingService::stop_recording() {
  if (recordingStatus == IDLE || recordingStatus == STOP)
    return 0;
  recordingStatus = STOP;

  if (videoCaptureThread.joinable())
    videoCaptureThread.join();

  if (audioCaptureThread.joinable())
    audioCaptureThread.join();

  if (capturedVideoPacketsProcessThread.joinable())
    capturedVideoPacketsProcessThread.join();

  if (capturedAudioPacketsProcessThread.joinable())
    capturedAudioPacketsProcessThread.join();

  if (recordingStatsThread.joinable())
    recordingStatsThread.join();

  videoTranscodeChain->flush();
  audioTranscodeChain->flush();

  if (av_write_trailer(outputMuxer->getContext()) < 0) {
    std::cout << "write error" << std::endl;
    return -1;
  }

  return 0;
}

/// Initializes all the structures needed for the recording process.
/// Accepts deviceAddresses as input in the following format:
///   deviceAddress: "{deviceID}:{url}"
///
/// Examples:
///   - MacOS
///       videoAddress:"avfoundation:1"
///       audioAddress:"avfoundation:1"
///   - Linux
///       videoAddress:"x11grab:...."
///       audioAddress:"pulse:....."
///   - Windows
///       videoAddress:"dshow:...."
///       audioAddress:"dshow:...."
RecordingService::RecordingService(RecordingConfig config) {
  recordingStatus = IDLE;

  // Initialize the LibAV devices
  avdevice_register_all();

  // Unpack device addresses
  std::string videoDeviceID, videoURL, audioDeviceID, audioURL;
  std::tie(videoDeviceID, videoURL) = unpackDeviceAddress(config.videoAddress);
  std::tie(audioDeviceID, audioURL) = unpackDeviceAddress(config.audioAddress);

  // Open A/V devices (demuxers)
  if (videoDeviceID == audioDeviceID) {
    mainDevice =
        DeviceContext::init_demuxer(videoDeviceID, videoURL, audioURL,
                                    get_device_options(videoDeviceID, config));
    auxDevice = mainDevice;
  } else {
    mainDevice = DeviceContext::init_demuxer(
        videoDeviceID, videoURL, "", get_device_options(videoDeviceID, config));
    auxDevice = DeviceContext::init_demuxer(
        audioDeviceID, "", audioURL, get_device_options(audioDeviceID, config));
  }

  // Init muxer
  outputMuxer = DeviceContext::init_muxer(config.outputFilename);

  // Init video rings
  auto* videoDecoderRing = new DecoderChainRing(mainDevice->getVideoStream());

  auto [outputWidth, outputHeight, originX, originY] =
      get_output_window(videoDecoderRing->getDecoderContext()->width,
                        videoDecoderRing->getDecoderContext()->height, config);
  EncoderConfig videoEncoderConfig = {
      .codecID = AV_CODEC_ID_H264,
      .codecType = AVMEDIA_TYPE_VIDEO,
      .encoderOptions = {{"profile", "main"},
                         {"preset", "ultrafast"},
                         {"x264-params",
                          "keyint=60:min-keyint=60:scenecut=0:force-cfr=1"},
                         {"tune", "zerolatency"}},
      .bitRate = OUTPUT_VIDEO_BIT_RATE,
      .height = outputHeight,
      .width = outputWidth,
      .pixelFormat = OUTPUT_VIDEO_PIXEL_FMT,
      .frameRate = av_guess_frame_rate(mainDevice->getContext(),
                                       mainDevice->getVideoStream(), nullptr)
                       .num};
  auto* videoEncoderRing =
      new EncoderChainRing(mainDevice->getVideoStream(),
                           outputMuxer->getVideoStream(), videoEncoderConfig);

  std::vector<FilterChainRing*> videoFilterRings;

  auto [scaledWidth, scaledHeight] = RecordingService::get_scaled_resolution(
      videoDecoderRing->getDecoderContext()->width,
      videoDecoderRing->getDecoderContext()->height, config.rescaleValue);
  SWScaleConfig swScaleConfig = {
      .inputWidth = videoDecoderRing->getDecoderContext()->width,
      .inputHeight = videoDecoderRing->getDecoderContext()->height,
      .inputPixelFormat = videoDecoderRing->getDecoderContext()->pix_fmt,
      .outputWidth = scaledWidth,
      .outputHeight = scaledHeight,
      .outputPixelFormat = videoEncoderRing->getEncoderContext()->pix_fmt,
  };
  auto* swScaleFilterRing = new SWScaleFilterRing(swScaleConfig);
  videoFilterRings.push_back(swScaleFilterRing);

  if (config.cropWindow) {
    VFCropConfig vfCropConfig = {
        .inputWidth = scaledWidth,
        .inputHeight = scaledHeight,
        .inputPixelFormat = videoEncoderRing->getEncoderContext()->pix_fmt,
        .inputTimeBase = mainDevice->getVideoStream()->time_base,
        .inputAspectRatio = mainDevice->getVideoStream()->sample_aspect_ratio,
        .originX = originX,
        .originY = originY,
        .outputWidth = outputWidth,
        .outputHeight = outputHeight,
        .outputPixelFormat = videoEncoderRing->getEncoderContext()->pix_fmt,
    };
    auto* vfCropFilterRing = new VFCropFilterRing(vfCropConfig);
    videoFilterRings.push_back(vfCropFilterRing);
  }

  // Init audio rings
  auto* audioDecoderRing = new DecoderChainRing(auxDevice->getAudioStream());

  int channels = auxDevice->getAudioStream()->codecpar->channels;
  EncoderConfig audioEncoderConfig = {
      .codecID = AV_CODEC_ID_AAC,
      .codecType = AVMEDIA_TYPE_AUDIO,
      .bitRate = OUTPUT_AUDIO_BIT_RATE,
      .channels = channels,
      .channelLayout = av_get_default_channel_layout(channels),
      .sampleRate = auxDevice->getAudioStream()->codecpar->sample_rate,
      .sampleFormat = OUTPUT_AUDIO_SAMPLE_FMT,
      .strictStdCompliance = FF_COMPLIANCE_NORMAL};
  auto* audioEncoderRing =
      new EncoderChainRing(auxDevice->getAudioStream(),
                           outputMuxer->getAudioStream(), audioEncoderConfig);

  SWResampleConfig swResampleConfig = {
      .inputChannels = audioDecoderRing->getDecoderContext()->channels,
      .inputChannelLayout = av_get_default_channel_layout(channels),
      .inputSampleFormat = audioDecoderRing->getDecoderContext()->sample_fmt,
      .inputSampleRate = audioDecoderRing->getDecoderContext()->sample_rate,
      .inputFrameSize = audioDecoderRing->getDecoderContext()->frame_size,
      .inputTimeBase = auxDevice->getAudioStream()->time_base,
      .outputChannels = audioEncoderRing->getEncoderContext()->channels,
      .outputChannelLayout = av_get_default_channel_layout(channels),
      .outputSampleFormat = audioEncoderRing->getEncoderContext()->sample_fmt,
      .outputSampleRate = audioEncoderRing->getEncoderContext()->sample_rate,
      .outputFrameSize = audioEncoderRing->getEncoderContext()->frame_size,
      .outputTimeBase = audioEncoderRing->getEncoderContext()->time_base,
  };
  auto* swResampleFilterRing = new SWResampleFilterRing(swResampleConfig);
  std::vector<FilterChainRing*> audioFilterRings = {swResampleFilterRing};

  // Init common rings
  auto* muxerRing = new MuxerChainRing(outputMuxer);

  // Init transcode process chains
  this->videoTranscodeChain = new ProcessChain(
      videoDecoderRing, videoFilterRings, videoEncoderRing, muxerRing);
  this->audioTranscodeChain = new ProcessChain(
      audioDecoderRing, audioFilterRings, audioEncoderRing, muxerRing);

  // Init control thread
  controlThread = std::thread([this]() {
    // Initialize signal to stop recording on sigterm
    std::signal(SIGTERM, [](int) {
      std::cout << "sigterm" << std::endl;
      mustTerminateSignal = true;
    });

    std::signal(SIGINT, [](int) {
      std::cout << "sigterm" << std::endl;
      mustTerminateSignal = true;
    });

    while (!mustTerminateSignal) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (mustTerminateSignal) {
      std::cout << "exiting" << std::endl;
      stop_recording();
      std::cout << "exited" << std::endl;
    }
  });
}

void RecordingService::wait_recording() {
  controlThread.join();
}
