/*
    device.h: ABC collecting various device-generic stuff

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "device.h"


Device::Device()
    : errno_(0)
{
}


Device::~Device()
{
}


