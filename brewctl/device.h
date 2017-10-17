#ifndef DEVICE_H_INC
#define DEVICE_H_INC
/*
    device.h: ABC collecting various device-generic stuff

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "error.h"
#include <string>


class Device
{
protected:
                        Device();
    virtual             ~Device();
};

#endif // DEVICE_H_INC

