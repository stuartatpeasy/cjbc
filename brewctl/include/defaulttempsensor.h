#ifndef DEFAULTTEMPSENSOR_H_INC
#define DEFAULTTEMPSENSOR_H_INC
/*
    defaulttempsensor.h: representation of the absence of a temperature sensor.  Any temperature measurements taken
    through objects of this type will return absolute zero.  Useful in cases where a ref/ptr to a TempSensor is
    required, but no sensor actually exists.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "error.h"
#include "log.h"
#include "temperature.h"
#include <memory>
#include <string>


class DefaultTempSensor
{
public:
                                DefaultTempSensor(const int channel = -1, const std::string& name = "DefaultTempSensor")
                                    noexcept;
    virtual                     ~DefaultTempSensor() noexcept;

                                DefaultTempSensor(const DefaultTempSensor& rhs) noexcept;
                                DefaultTempSensor(DefaultTempSensor&& rhs) noexcept;

    virtual DefaultTempSensor&  operator=(const DefaultTempSensor& rhs) noexcept;
    virtual DefaultTempSensor&  operator=(DefaultTempSensor&& rhs) noexcept;

    virtual Temperature         sense(Error * const err = nullptr) noexcept;
    virtual bool                inRange() noexcept { return false; };
    virtual std::string         name() const noexcept { return name_; };
    int                         channel() const noexcept { return channel_; };

protected:
    virtual void                move(DefaultTempSensor& rhs) noexcept;
    virtual void                copy(const DefaultTempSensor& rhs) noexcept;

    int                         channel_;
    std::string                 name_;
};


typedef std::unique_ptr<DefaultTempSensor> DefaultTempSensor_uptr_t;


#endif // DEFAULTTEMPSENSOR_H_INC

