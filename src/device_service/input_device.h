//
// Created by Marco Bramini on 16/09/21.
//

#ifndef PDS_SCREEN_RECORDING_INPUT_DEVICE_H
#define PDS_SCREEN_RECORDING_INPUT_DEVICE_H

#include <utility>
#include <iostream>
class InputDevice {
    std::string id;
    std::string name;

public:
    InputDevice(std::string id, std::string name) : id(std::move(id)), name(std::move(name)) {};

    std::string getID() { return this->id; };
    std::string getName() { return this->name; };

    void toString(){std::cout<<"ID: "<<this->id<<" Name:"<<this->name<<std::endl;};
};


class InputDeviceAudio: public InputDevice{
public:
    InputDeviceAudio(const std::string &id, const std::string &name) : InputDevice(id, name) {}
};

class InputDeviceVideo: public InputDevice{
    int x,y;
public:
    InputDeviceVideo(const std::string &id, const std::string &name, int x, int y) : InputDevice(id, name), x(x),y(y) {}
};

#endif //PDS_SCREEN_RECORDING_INPUT_DEVICE_H
