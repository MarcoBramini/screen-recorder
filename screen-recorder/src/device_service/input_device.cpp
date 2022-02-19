#include "input_device.h"
#include <fmt/core.h>

std::string InputDevice::getURL() {
    return this->url;
}

std::string InputDevice::getName() {
    return this->name;
}

std::string InputDevice::getDeviceID() {
    return this->deviceID;
}

std::string InputDevice::getDeviceAddress() {
    return fmt::format("{}:{}", deviceID, url);
}

std::string InputDevice::toString() {
    return fmt::format("URL: {} | DeviceID: {} | Name: {}", this->url, this->deviceID, this->name);
}

