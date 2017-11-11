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
#include <ctime>
#include <memory>
#include <string>


class TempSensor
{
public:
                        TempSensor(const int thermistor_id, const int channel, Error * const err = nullptr) noexcept;
    virtual             ~TempSensor() noexcept;

                        TempSensor(const TempSensor& rhs) = delete;
                        TempSensor(TempSensor&& rhs) noexcept;

    TempSensor&         operator=(const TempSensor& rhs) = delete;
    TempSensor&         operator=(TempSensor&& rhs) noexcept;

    virtual Temperature sense(Error * const err = nullptr) noexcept;
    virtual bool        inRange() const noexcept;
    virtual std::string name() const noexcept { return name_; };

    static TempSensor * getTempSensor(const int sessionId, const std::string& role, Error * const err = nullptr)
                            noexcept;
    static TempSensor * getSessionVesselTempSensor(const int sessionId, Error * const err = nullptr) noexcept;
    static TempSensor * getAmbientTempSensor(Error * const err = nullptr) noexcept;

protected:
    double              readRaw(Error * const err = nullptr) noexcept;
    void                move(TempSensor& rhs) noexcept;
    void                writeTempLog();

    int                 channel_;
    Thermistor *        thermistor_;
    int                 nsamples_;
    double              Idrive_;
    bool                sampleTaken_;
    std::string         name_;
    Temperature         currentTemp_;
    Temperature         rangeMin_;
    Temperature         rangeMax_;
    time_t              lastLogWriteTime_;
    int                 logInterval_;
};

#endif // TEMPSENSOR_H_INC

