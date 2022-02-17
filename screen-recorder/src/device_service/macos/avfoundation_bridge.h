//
// Created by Marco Bramini on 16/09/21.
//


#ifndef PDS_SCREEN_RECORDING_AVFOUNDATION_BRIDGE_H
#define PDS_SCREEN_RECORDING_AVFOUNDATION_BRIDGE_H

#include "../input_device.h"

int avfoundation_list_video_devices(std::vector<InputDeviceVideo> *devices);
int avfoundation_list_audio_devices(std::vector<InputDeviceAudio> *devices);

#endif //PDS_SCREEN_RECORDING_AVFOUNDATION_BRIDGE_H
