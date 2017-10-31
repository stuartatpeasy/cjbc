#ifndef SENSOR_H_INC
#define SENSOR_H_INC
/*
    sensor.h: sensor abstraction.  Considers a sensor to be a device connected to a specified ADC channel, with a
    specified constant current flowing through it.  Enables derived classes to read the voltage on the channel.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "error.h"
#include <memory>


class Sensor
{
public:
                        Sensor(const int channel);
protected:
    bool                readRaw(double& voltage, Error * const err = nullptr);

    const int           channel_;
    double              vref_;
};

#endif // SENSOR_H_INC
