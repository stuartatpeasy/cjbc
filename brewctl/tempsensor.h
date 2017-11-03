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
#include "tempsensorinterface.h"
#include "thermistor.h"
#include <memory>


class TempSensor : public TempSensorInterface
{
public:
                                    TempSensor(const int thermistor_id, const int channel, Error * const err = nullptr)
                                        noexcept;

    Temperature                     sense(Error * const err = nullptr) noexcept;

    static TempSensorInterface *    getSessionVesselTempSensor(const int sessionId, Error * const err = nullptr)
                                        noexcept;

protected:
    Thermistor *                    thermistor_;

    int                             nsamples_;
    double                          Idrive_;
    bool                            sampleTaken_;
};

#endif // TEMPSENSOR_H_INC

