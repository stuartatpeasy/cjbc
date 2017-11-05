#ifndef NULLTEMPSENSOR_H_INC
#define NULLTEMPSENSOR_H_INC
/*
    nulltempsensor.h: representation of the absence of a temperature sensor.  Any temperature measurements taken
    through objects of this type will return absolute zero.  Useful in cases where a ref/ptr to a TempSensor is
    required, but no sensor actually exists.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "error.h"
#include "temperature.h"
#include "tempsensor.h"
#include <memory>
#include <string>


class NullTempSensor : public TempSensor
{
public:
                        NullTempSensor(const int channel = -1) noexcept;
    virtual             ~NullTempSensor() noexcept;

    virtual Temperature sense(Error * const err = nullptr) noexcept;
    virtual bool        inRange() const noexcept { return false; };
};

#endif // NULLTEMPSENSOR_H_INC

