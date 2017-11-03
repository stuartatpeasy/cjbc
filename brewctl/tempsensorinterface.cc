/*
    tempsensorinterface.cc: temperature sensor abstraction.  Considers a sensor to be a device connected to a specified
    ADC channel, with a specified constant current flowing through it.  Enables derived classes to read the voltage on
    the channel.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "tempsensorinterface.h"
#include "registry.h"


TempSensorInterface::TempSensorInterface(const int channel)
    : channel_(channel), tempKelvin_(0.0)
{
}


TempSensorInterface::~TempSensorInterface()
{
}


double TempSensorInterface::readRaw(Error * const err)
{
    return Registry::instance().adc().read(channel_, err);
}

