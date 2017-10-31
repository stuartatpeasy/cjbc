#ifndef TEMPERATURESENSOR_H_INC
#define TEMPERATURESENSOR_H_INC
/*
    temperaturesensor.h: models a temperature sensor (i.e. a thermistor) attached to an ADC channel.
    Assumes that there is a constance current flowing through the sensor and developing a voltage
    across it.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "sqlite.h"
#include "error.h"
#include "sensor.h"
#include "thermistor.h"
#include <memory>
#include <string>


class TemperatureSensor : public Sensor
{
public:
                    TemperatureSensor(const int thermistor_id, const int channel,
                                      Error * const err = nullptr);

    bool            sense(Temperature& T, Error * const err = nullptr);

protected:
    const int       thermistor_id_;
    std::string     name_;
    Thermistor *    thermistor_;

    int             nsamples_;
    double          Idrive_;
    double          tempKelvin_;
    bool            sampleTaken_;
};


#endif // TEMPERATURESENSOR_H_INC

