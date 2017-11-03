#ifndef NULLTEMPSENSOR_H_INC
#define NULLTEMPSENSOR_H_INC
/*
    nulltempsensor.h: represents the absence of a temperature sensor; operations performed on objects of this type have
    no effect, and the "sensor" will always return a temperature of absolute zero.  Used in cases where ptrs/refs to a
    TempSensorInterface-derived object are required, but a real sensor may not be present.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "temperature.h"
#include "tempsensorinterface.h"


class NullTempSensor : public TempSensorInterface
{
public:
                NullTempSensor() noexcept;

    Temperature sense(Error * const err = nullptr) noexcept;
};

#endif // NULLTEMPSENSOR_H_INC

