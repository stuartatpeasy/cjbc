#ifndef TEMPSENSOR_H_INC
#define TEMPSENSOR_H_INC
/*
    tempsensor.h: models a temperature sensor (i.e. a thermistor) attached to an ADC channel.  Assumes that there is a
    constant current flowing through the sensor and developing a voltage across it.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "sqlite.h"
#include "error.h"
#include "temperature.h"
#include "thermistor.h"
#include <memory>
#include <string>


class TempSensor
{
public:
                        TempSensor(const int thermistor_id, const int channel, Error * const err = nullptr) noexcept;
    virtual             ~TempSensor() noexcept;

    virtual Temperature sense(Error * const err = nullptr) noexcept;
    virtual std::string name() const noexcept { return name_; };

    static TempSensor * getSessionVesselTempSensor(const int sessionId, Error * const err = nullptr) noexcept;

protected:
    double              readRaw(Error * const err = nullptr);

    const int           channel_;
    Thermistor *        thermistor_;
    int                 nsamples_;
    double              Idrive_;
    bool                sampleTaken_;
    double              tempKelvin_;
    std::string         name_;
};

#endif // TEMPSENSOR_H_INC

