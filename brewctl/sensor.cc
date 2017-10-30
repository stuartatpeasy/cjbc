/*
    sensor.cc: sensor abstraction.  Considers a sensor to be a device connected to a specified ADC
    channel, with a specified constant current flowing through it.  Enables derived classes to read
    the voltage on the channel.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "sensor.h"


Sensor::Sensor(ADC& adc, const int channel, const Config& config)
    : adc_(adc), channel_(channel), config_(config)
{
    
}


bool Sensor::readRaw(double& voltage, Error * const err)
{
    return adc_.read(channel_, voltage, err);
}

