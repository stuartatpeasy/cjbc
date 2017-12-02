#ifndef PERIPHERALS_TEMPSENSOR_H_INC
#define PERIPHERALS_TEMPSENSOR_H_INC
/*
    tempsensor.h: models a temperature sensor (i.e. a thermistor) attached to an ADC channel.  Assumes that there is a
    constant current flowing through the sensor and developing a voltage across it.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "include/framework/error.h"
#include "include/peripherals/defaulttempsensor.h"
#include "include/temperature.h"
#include "include/thermistor.h"
#include <ctime>
#include <memory>
#include <mutex>
#include <string>


class TempSensor : public DefaultTempSensor
{
public:
                                    TempSensor(const int thermistor_id, const int channel, Error * const err = nullptr)
                                        noexcept;
    virtual                         ~TempSensor() noexcept;

                                    TempSensor(const TempSensor& rhs) = delete;
                                    TempSensor(TempSensor&& rhs) noexcept;

    virtual TempSensor&             operator=(const TempSensor& rhs) = delete;
    virtual TempSensor&             operator=(TempSensor&& rhs) noexcept;

    virtual Temperature             sense(Error * const err = nullptr) noexcept;
    virtual bool                    inRange() noexcept;

    static DefaultTempSensor_uptr_t getSessionVesselTempSensor(const int sessionId, Error * const err = nullptr)
                                        noexcept;
    static DefaultTempSensor_uptr_t getAmbientTempSensor(Error * const err = nullptr) noexcept;

protected:
    static DefaultTempSensor_uptr_t getTempSensor(const int sessionId, const std::string& role,
                                                  Error * const err = nullptr) noexcept;
    double                          readRaw(Error * const err = nullptr) noexcept;
    void                            move(TempSensor& rhs) noexcept;
    void                            writeTempLog();

    Thermistor *                    thermistor_;
    int                             avglen_;
    int                             nsamples_;
    double                          Idrive_;
    Temperature                     currentTemp_;
    Temperature                     rangeMin_;
    Temperature                     rangeMax_;
    time_t                          lastLogWriteTime_;
    int                             logInterval_;
    std::mutex                      lock_;
};

#endif // PERIPHERALS_TEMPSENSOR_H_INC

