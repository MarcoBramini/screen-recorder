#ifndef PDS_SCREEN_RECORDING_AVFOUNDATION_BRIDGE_H
#define PDS_SCREEN_RECORDING_AVFOUNDATION_BRIDGE_H

#include "../input_device.h"

int avfoundation_list_video_devices(std::vector<InputDeviceVideo> *devices);
int avfoundation_list_audio_devices(std::vector<InputDeviceAudio> *devices);

int check_screen_capture_permissions();
int check_camera_capture_permissions();
int check_microphone_capture_permissions();

void request_screen_capture_permissions();
void request_camera_capture_permissions();
void request_microphone_capture_permissions();

#endif //PDS_SCREEN_RECORDING_AVFOUNDATION_BRIDGE_H
