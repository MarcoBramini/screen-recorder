#ifndef PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
#define PDS_SCREEN_RECORDING_RECORDINGSERVICE_H

#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include "device_context.h"
#include "process_chain/process_chain.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

struct RecordingConfig {
  std::string videoAddress;
  std::string audioAddress;
  std::string outputFilename;
  std::optional<std::tuple<int, int, int, int>>
      cropWindow;            // x1,y1,x2,y2 from top left
  float rescaleValue = 1.0;  // output resolution will be calculated as
                             // (height,width)//rescaleValue
  int framerate = 30;
};

// Settings
const AVSampleFormat OUTPUT_AUDIO_SAMPLE_FMT = AV_SAMPLE_FMT_FLTP;
const AVPixelFormat OUTPUT_VIDEO_PIXEL_FMT = AV_PIX_FMT_YUV420P;
const int64_t OUTPUT_VIDEO_BIT_RATE = 1500000;
const int64_t OUTPUT_AUDIO_BIT_RATE = 96000;

enum RecordingStatus { IDLE, RECORDING, PAUSE, STOP };

class RecordingService {
  // ------
  // Status
  // ------
  RecordingStatus recordingStatus;

  // -------
  // Threads
  // -------

  std::thread videoCaptureThread;
  std::thread audioCaptureThread;
  std::thread capturedVideoPacketsProcessThread;
  std::thread capturedAudioPacketsProcessThread;
  std::thread recordingStatsThread;
  std::thread controlThread;

  // ------
  // Input
  // ------

  // Input context
  DeviceContext* mainDevice;
  DeviceContext* auxDevice;

  // ------
  // Output
  // ------

  // Output context
  DeviceContext* outputMuxer;

  // ---------------
  // Transcode Chain
  // ---------------

  ProcessChain* videoTranscodeChain;
  ProcessChain* audioTranscodeChain;

  // recording_utils.cpp
  static std::map<std::string, std::string> get_device_options(
      const std::string& deviceID,
      RecordingConfig config);

  static std::tuple<std::string, std::string> unpackDeviceAddress(
      const std::string& deviceAddress);

  static std::tuple<int, int> get_scaled_resolution(int inputWidth,
                                                    int inputHeight,
                                                    float scale);

  static std::tuple<int, int, int, int>
  get_output_window(int inputWidth, int inputHeight, RecordingConfig config);

  // recording_service.cpp
  int start_capture_loop(DeviceContext* inputDevice);

  void start_transcode_process(ProcessChain* transcodeChain);

 public:
  RecordingService(RecordingConfig config);

  int start_recording();

  int pause_recording();

  int resume_recording();

  int stop_recording();

  void enqueue_video_packet(DeviceContext* inputDevice,
                            AVPacket* inputVideoPacket);

  void enqueue_audio_packet(DeviceContext* inputDevice,
                            AVPacket* inputAudioPacket);

  void rec_stats_loop();

  void wait_recording();

  ~RecordingService() {
    if (mainDevice != auxDevice) {
      delete auxDevice;
    }
    delete mainDevice;
    delete outputMuxer;

    delete videoTranscodeChain;
    delete audioTranscodeChain;
  };
};

#endif  // PDS_SCREEN_RECORDING_RECORDINGSERVICE_H
