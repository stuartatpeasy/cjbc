/*
    nulltempsensor.cc: representation of the absence of a temperature sensor.  Any temperature measurements taken
    through objects of this type will return absolute zero.  Useful in cases where a ref/ptr to a TempSensor is
    required, but no sensor actually exists.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "nulltempsensor.h"
#include "log.h"
#include "registry.h"


NullTempSensor::NullTempSensor(const int channel) noexcept
    : TempSensor(-1, channel)
{
}


NullTempSensor::~NullTempSensor() noexcept
{
}


// sense() - always return a temperature corresponding to absolute zero.
//
Temperature NullTempSensor::sense(Error * const err) noexcept
{
    (void) err;     // Suppress "unused arg" warning
    return Temperature();
}

