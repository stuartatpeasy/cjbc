/*
    sensor.cc: sensor abstraction.  Considers a sensor to be a device connected to a specified ADC channel, with a
    specified constant current flowing through it.  Enables derived classes to read the voltage on the channel.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "sensor.h"
#include "registry.h"


Sensor::Sensor(const int channel)
    : channel_(channel)
{
    
}


bool Sensor::readRaw(double& voltage, Error * const err)
{
    return Registry::instance().adc().read(channel_, voltage, err);
}

