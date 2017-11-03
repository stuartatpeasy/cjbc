/*
    nulltempsensor.cc: represents the absence of a temperature sensor; operations performed on objects of this type have
    no effect, and the "sensor" will always return a temperature of absolute zero.  Used in cases where ptrs/refs to a
    TempSensorInterface-derived object are required, but a real sensor may not be present.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "nulltempsensor.h"


NullTempSensor::NullTempSensor() noexcept
    : TempSensorInterface(-1)
{
}


Temperature NullTempSensor::sense(Error * const err) noexcept
{
    (void) err;         // Suppress warning about unused arg
    return Temperature();
}

