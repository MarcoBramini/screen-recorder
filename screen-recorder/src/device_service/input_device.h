#ifndef PDS_SCREEN_RECORDING_INPUT_DEVICE_H
#define PDS_SCREEN_RECORDING_INPUT_DEVICE_H

#include <utility>
#include <iostream>
#include <fmt/core.h>

class InputDevice {
    std::string url;
    std::string deviceID;
    std::string name;

public:
    InputDevice(std::string url, std::string deviceID, std::string name) : url(std::move(url)),
                                                                           deviceID(std::move(deviceID)),
                                                                           name(std::move(name)) {};

    std::string getURL();

    std::string getName();

    std::string getDeviceID();

    std::string getDeviceAddress();


    std::string toString();
};


class InputDeviceAudio : public InputDevice {
public:
    InputDeviceAudio(const std::string &url, const std::string &deviceID, const std::string &name) : InputDevice(url,
                                                                                                                 deviceID,
                                                                                                                 name) {};
};

class InputDeviceVideo : public InputDevice {
    float x, y, width, height;
    bool primary;
    std::string port;
public:
    InputDeviceVideo(const std::string &url, const std::string &deviceID, const std::string &name, float x, float y,
                     float width, float height, bool primary, std::string port) : InputDevice(url, deviceID, name),
                                                                                  x(x), y(y), width(width),
                                                                                  height(height), primary(primary),
                                                                                  port(std::move(port)) {};
};

#endif //PDS_SCREEN_RECORDING_INPUT_DEVICE_H
