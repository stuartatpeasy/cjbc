#ifndef TEMPSENSORINTERFACE_H_INC
#define TEMPSENSORINTERFACE_H_INC
/*
    sensor.h: sensor abstraction.  Considers a sensor to be a device connected to a specified ADC channel, with a
    specified constant current flowing through it.  Enables derived classes to read the voltage on the channel.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "error.h"
#include "temperature.h"
#include <memory>
#include <string>


class TempSensorInterface
{
protected:
                        TempSensorInterface(const int channel);
public:
    virtual             ~TempSensorInterface();

    virtual Temperature sense(Error * const err = nullptr) noexcept = 0;
    virtual std::string name() const noexcept { return name_; };

protected:
    double              readRaw(Error * const err = nullptr);

    const int           channel_;
    double              vref_;
    std::string         name_;
    double              tempKelvin_;
};

#endif // TEMPSENSORINTERFACE_H_INC

