#include "recording_service_impl.h"

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

using namespace std::chrono;

/// Handles a captured packed, enqueueing it in the transcoder packets queue.
/// It is used as callback for the PacketCapturer objects.
void on_packet_capture(std::unique_ptr<AVPacket, FFMpegObjectsDeleter> packet,
                       int64_t relativePts,
                       ProcessChain& transcodeChain,
                       std::mutex& queueMutex,
                       std::condition_variable& processChainCV) {
  {
    std::lock_guard<std::mutex> lk(queueMutex);
    transcodeChain.enqueueSourcePacket(std::move(packet), relativePts);
  }
  processChainCV.notify_all();
}

/// Starts the packet capture loop.
/// It temporarily stops if the recording process is paused.
/// The loop exits when the recording proces is stopped.
void RecordingServiceImpl::start_capture_loop(PacketCapturer& capturer) {
  while (true) {
    {
      std::unique_lock<std::mutex> lock(recordingStatusMutex);
      captureCV.wait(lock, [&] { return recordingStatus != PAUSE; });

      if (recordingStatus == STOP)
        break;
    }

    capturer.capture_next();
    capturer.sleep();
  }
}

/// Processes all the captured packets in the a/v transcoder packets queue.
/// It temporarily stops if the queue is empty but the recording process is
/// still active (e.g. paused). The loop exits when the queue is empty and the
/// recording process is stopped.
void RecordingServiceImpl::start_transcode_process(
    ProcessChain& transcodeChain,
    std::mutex& queueMutex,
    std::condition_variable& queueCV) {
  bool mustStop = false;
  while (true) {
    // Check if the recording process has been stopped
    {
      std::lock_guard<std::mutex> lk(recordingStatusMutex);
      mustStop = recordingStatus == STOP;
    }

    // Process the next packet until the queue is empty.
    // If the queue is empty, wait for new packets but check every 100ms if the
    // recording has been stopped.
    {
      std::unique_lock<std::mutex> lock(queueMutex);
      queueCV.wait_for(lock, 100ms, [&transcodeChain] {
        return !transcodeChain.isSourceQueueEmpty();
      });

      if (transcodeChain.isSourceQueueEmpty() && mustStop)
        break;

      if (transcodeChain.isSourceQueueEmpty())
        continue;
    }

    transcodeChain.processNext();
  }
}

/// Starts the recording process.
/// It writes the output file header and starts all the needed sub processes.
void RecordingServiceImpl::start_recording() {
  int ret = avformat_write_header(outputMuxer->getContext(), nullptr);
  if (ret < 0) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {},
        fmt::format("error writing the output file header ({})",
                    Error::unpackAVError(ret))));
  }

  recordingStatus = RECORDING;
  startTimestamp =
      duration_cast<microseconds>(system_clock::now().time_since_epoch())
          .count();

  // ---------------
  // Video recording
  // ---------------

  mainDeviceCaptureThread =
      std::thread([this]() { start_capture_loop(*mainDeviceCapturer); });

  capturedVideoPacketsProcessThread = std::thread([this]() {
    start_transcode_process(*videoTranscodeChain, videoProcessChainQueueMutex,
                            videoProcessChainCV);
  });

  // ---------------
  // Audio recording
  // ---------------

  if (!isAudioDisabled) {
    if (mainDevice != auxDevice) {
      auxDeviceCaptureThread =
          std::thread([this]() { start_capture_loop(*auxDeviceCapturer); });
    }

    capturedAudioPacketsProcessThread = std::thread([this]() {
      start_transcode_process(*audioTranscodeChain, audioProcessChainQueueMutex,
                              audioProcessChainCV);
    });
  }
}

/// Pause the recording process
void RecordingServiceImpl::pause_recording() {
  // Set pauseTimestamp
  pauseTimestamp =
      duration_cast<microseconds>(system_clock::now().time_since_epoch())
          .count();

  {
    std::lock_guard<std::mutex> lk(recordingStatusMutex);
    recordingStatus = PAUSE;
  }
  captureCV.notify_all();
}

/// Resume the recording process
void RecordingServiceImpl::resume_recording() {
  // Increment pausedTime by resumeTimestamp - pauseTimestamp interval
  int64_t resumeTimestamp =
      duration_cast<microseconds>(system_clock::now().time_since_epoch())
          .count();

  mainDeviceCapturer->add_pause_duration(resumeTimestamp - pauseTimestamp);
  if (mainDevice != auxDevice && !isAudioDisabled) {
    auxDeviceCapturer->add_pause_duration(resumeTimestamp - pauseTimestamp);
  }

  {
    std::lock_guard<std::mutex> lk(recordingStatusMutex);
    recordingStatus = RECORDING;
  }
  captureCV.notify_all();
}

/// Stops the recording process
/// Waits for the sub-processes to end. Remaining captured packets are flushed.
/// Finally, it writes the output file trailer.
void RecordingServiceImpl::stop_recording() {
  if (recordingStatus == IDLE || recordingStatus == STOP)
    return;

  {
    std::lock_guard<std::mutex> lock(recordingStatusMutex);
    recordingStatus = STOP;
  }

  stopTimestamp =
      duration_cast<microseconds>(system_clock::now().time_since_epoch())
          .count();

  if (mainDeviceCaptureThread.joinable())
    mainDeviceCaptureThread.join();

  if (auxDeviceCaptureThread.joinable())
    auxDeviceCaptureThread.join();

  if (capturedVideoPacketsProcessThread.joinable())
    capturedVideoPacketsProcessThread.join();

  if (capturedAudioPacketsProcessThread.joinable())
    capturedAudioPacketsProcessThread.join();

  videoTranscodeChain->flush();
  if (!isAudioDisabled) {
    audioTranscodeChain->flush();
  }

  int ret = av_write_trailer(outputMuxer->getContext());
  if (ret < 0) {
    throw std::runtime_error(Error::build_error_message(
        __FUNCTION__, {},
        fmt::format("error writing the output file trailer ({})",
                    Error::unpackAVError(ret))));
  }
}

/// Initializes all the structures needed for the recording process
RecordingServiceImpl::RecordingServiceImpl(const RecordingConfig& config) {
  recordingStatus = IDLE;
  startTimestamp = 0;
  pauseTimestamp = 0;
  stopTimestamp = 0;

  // Initialize the LibAV devices
  avdevice_register_all();

  // Unpack device addresses
  std::string videoDeviceID, videoURL, audioDeviceID, audioURL;
  std::tie(videoDeviceID, videoURL) =
      unpackDeviceAddress(config.getVideoAddress());
  std::tie(audioDeviceID, audioURL) =
      unpackDeviceAddress(config.getAudioAddress());
  isAudioDisabled = audioDeviceID.empty();

  // -----------
  // A/V Devices
  // -----------

  // Open A/V devices (demuxers).
  // Video device address is mandatory. Audio device address is optional, when
  // audio is disabled. Main device always holds the video stream while aux
  // device can hold the audio stream, if not disabled. AVFoundation also embed
  // the audio stream in the same device: in this case the main and the aux
  // devices are the same.
  if (videoDeviceID == audioDeviceID) {
    mainDevice =
        DeviceContext::init_demuxer(videoDeviceID, videoURL, audioURL,
                                    get_device_options(videoDeviceID, config));
    auxDevice = mainDevice;
  } else {
    mainDevice = DeviceContext::init_demuxer(
        videoDeviceID, videoURL, "", get_device_options(videoDeviceID, config));
    if (!isAudioDisabled) {
      auxDevice = DeviceContext::init_demuxer(
          audioDeviceID, "", audioURL,
          get_device_options(audioDeviceID, config));
    }
  }

  // ------------------
  // A/V Process Chains
  // ------------------

  // Init muxer
  outputMuxer =
      DeviceContext::init_muxer(config.getOutputPath(), isAudioDisabled);

  // Init common rings
  auto muxerRing = std::make_shared<MuxerChainRing>(outputMuxer);

  // Init video rings
  auto videoDecoderRing =
      std::make_shared<DecoderChainRing>(mainDevice->getVideoStream());

  auto [encoderOutputWidth, encoderOutputHeight, scalerOutputWidth,
        scalerOutputHeight, cropOriginX, cropOriginY] =
      get_output_image_parameters(videoDecoderRing->getDecoderContext()->width,
                                  videoDecoderRing->getDecoderContext()->height,
                                  config);

  EncoderConfig videoEncoderConfig = {
      .codecID = AV_CODEC_ID_H264,
      .codecType = AVMEDIA_TYPE_VIDEO,
      .encoderOptions = {{"profile", "main"},
                         {"preset", "ultrafast"},
                         {"x264-params",
                          "keyint=60:min-keyint=60:scenecut=0:force-cfr=1"},
                         {"tune", "zerolatency"}},
      .bitRate = OUTPUT_VIDEO_BIT_RATE,
      .height = encoderOutputHeight,
      .width = encoderOutputWidth,
      .pixelFormat = OUTPUT_VIDEO_PIXEL_FMT,
      .frameRate = av_guess_frame_rate(mainDevice->getContext(),
                                       mainDevice->getVideoStream(), nullptr)
                       .num};
  auto videoEncoderRing = std::make_shared<EncoderChainRing>(
      mainDevice->getVideoStream(), outputMuxer->getVideoStream(),
      videoEncoderConfig);

  std::vector<std::shared_ptr<FilterChainRing>> videoFilterRings;

  SWScaleConfig swScaleConfig = {
      .inputWidth = videoDecoderRing->getDecoderContext()->width,
      .inputHeight = videoDecoderRing->getDecoderContext()->height,
      .inputPixelFormat = videoDecoderRing->getDecoderContext()->pix_fmt,
      .outputWidth = scalerOutputWidth,
      .outputHeight = scalerOutputHeight,
      .outputPixelFormat = videoEncoderRing->getEncoderContext()->pix_fmt,
  };
  auto swScaleFilterRing = std::make_shared<SWScaleFilterRing>(swScaleConfig);
  videoFilterRings.push_back(swScaleFilterRing);

  if (config.getCaptureRegion()) {
    VFCropConfig vfCropConfig = {
        .inputWidth = scalerOutputWidth,
        .inputHeight = scalerOutputHeight,
        .inputPixelFormat = videoEncoderRing->getEncoderContext()->pix_fmt,
        .inputTimeBase = mainDevice->getVideoStream()->time_base,
        .inputAspectRatio = mainDevice->getVideoStream()->sample_aspect_ratio,
        .originX = cropOriginX,
        .originY = cropOriginY,
        .outputWidth = encoderOutputWidth,
        .outputHeight = encoderOutputHeight,
        .outputPixelFormat = videoEncoderRing->getEncoderContext()->pix_fmt,
    };
    auto vfCropFilterRing = std::make_shared<VFCropFilterRing>(vfCropConfig);
    videoFilterRings.push_back(vfCropFilterRing);
  }

  // Init video transcode process chain
  this->videoTranscodeChain = std::make_unique<ProcessChain>(
      videoDecoderRing, videoFilterRings, videoEncoderRing, muxerRing);

  if (!isAudioDisabled) {
    // Init audio rings
    auto audioDecoderRing =
        std::make_shared<DecoderChainRing>(auxDevice->getAudioStream());

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
    auto audioEncoderRing = std::make_shared<EncoderChainRing>(
        auxDevice->getAudioStream(), outputMuxer->getAudioStream(),
        audioEncoderConfig);

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
    auto swResampleFilterRing =
        std::make_shared<SWResampleFilterRing>(swResampleConfig);
    std::vector<std::shared_ptr<FilterChainRing>> audioFilterRings = {
        swResampleFilterRing};

    // Init audio transcode process chain
    this->audioTranscodeChain = std::make_unique<ProcessChain>(
        audioDecoderRing, audioFilterRings, audioEncoderRing, muxerRing);
  }

  // Init packet capturers
  auto onVideoPacketCaptureCallback =
      [this](std::unique_ptr<AVPacket, FFMpegObjectsDeleter> videoPacket,
             int64_t relativePts) {
        return on_packet_capture(
            std::move(videoPacket), relativePts, *videoTranscodeChain,
            videoProcessChainQueueMutex, videoProcessChainCV);
      };

  auto onAudioPacketCaptureCallback =
      [this](std::unique_ptr<AVPacket, FFMpegObjectsDeleter> audioPacket,
             int64_t relativePts) {
        return on_packet_capture(
            std::move(audioPacket), relativePts, *audioTranscodeChain,
            audioProcessChainQueueMutex, audioProcessChainCV);
      };

  mainDeviceCapturer = std::make_unique<PacketCapturer>(
      mainDevice, onVideoPacketCaptureCallback, onAudioPacketCaptureCallback);

  if (mainDevice != auxDevice && !isAudioDisabled) {
    auxDeviceCapturer = std::make_unique<PacketCapturer>(
        auxDevice, onVideoPacketCaptureCallback, onAudioPacketCaptureCallback);
  }

  // Init control thread
  useControlThread = config.isUseControlThread();
  if (useControlThread) {
    controlThread = std::thread([this]() {
      std::cout << "Tap Pause(p), Resume(r) or Stop(s)" << std::endl;
      char c;
      while (true) {
        scanf("%c", &c);
        if (c == 'p') {
          pause_recording();
          std::cout << "Paused" << std::endl;
        } else if (c == 'r') {
          resume_recording();
          std::cout << "Resumed" << std::endl;
        } else if (c == 's') {
          stop_recording();
          std::cout << "Stopped" << std::endl;
          break;
        }
      }
    });
  }
}

/// Wait for the control thread to return.
/// Must be only used when useControlThread is enabled.
void RecordingServiceImpl::wait_recording() {
  if (useControlThread)
    controlThread.join();
}

/// Returns information about the currently active recording
RecordingStats RecordingServiceImpl::get_recording_stats() {
  int64_t duration = 0;
  if (recordingStatus == RECORDING || recordingStatus == PAUSE) {
    int64_t nowTimestamp =
        duration_cast<microseconds>(system_clock::now().time_since_epoch())
            .count();
    duration = nowTimestamp - startTimestamp -
               mainDeviceCapturer->get_pause_duration();
  } else if (recordingStatus == STOP) {
    duration = stopTimestamp - startTimestamp -
               mainDeviceCapturer->get_pause_duration();
  }

  return {.status = recordingStatus, .recordingDuration = duration / 1000000};
}
