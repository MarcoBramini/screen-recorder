#ifndef SCREEN_RECORDER_RECORDING_SERVICE_H
#define SCREEN_RECORDER_RECORDING_SERVICE_H

#include "../src/recording_service/recording_service_impl.h"

class RecordingService {
    std::unique_ptr<RecordingServiceImpl> impl;
public:
    explicit RecordingService(const RecordingConfig &config) : impl(std::make_unique<RecordingServiceImpl>(config)) {};

    int start_recording() { return impl->start_recording(); };

    int pause_recording() { return impl->pause_recording(); };

    int resume_recording() { return impl->resume_recording(); };

    int stop_recording() { return impl->stop_recording(); };

    void wait_recording() { return impl->wait_recording(); };

    RecordingStats get_recording_stats() { return impl->get_recording_stats(); };
};

#endif //SCREEN_RECORDER_RECORDING_SERVICE_H
