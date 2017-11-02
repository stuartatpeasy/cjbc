/*
    temperaturesensor.h: models a temperature sensor (i.e. a thermistor) attached to an ADC channel.  Assumes that there
    is a constance current flowing through the sensor and developing a voltage across it.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "temperaturesensor.h"
#include "registry.h"
#include "sqlitestmt.h"


#define DEFAULT_MOVING_AVERAGE_LEN      (1000)  // Default length of moving average for sensor readings


TemperatureSensor::TemperatureSensor(const int thermistor_id, const int channel, Error * const err)
    : Sensor(channel), thermistor_(nullptr), tempKelvin_(0.0), sampleTaken_(false)
{
    // Initialise: read sensor data from the database
    SQLite& db = Registry::instance().db();
    SQLiteStmt thermistor_data;

    if(!db.prepare("SELECT name, type, Tref_C, Rref, beta, range_min, range_max "
                   "FROM thermistor WHERE id=:id", thermistor_data, err)
       || !thermistor_data.bind(":id", thermistor_id)
       || !thermistor_data.step(err))
        return;

    if(thermistor_data["type"].asString() != "thermistor")
    {
        formatError(err, SENSOR_INVALID_TYPE, thermistor_data["type"].asCString());
        return;
    }

    name_ = thermistor_data["name"].asString();
    
    thermistor_ = new Thermistor(thermistor_data["beta"], thermistor_data["Rref"],
                                 Temperature(thermistor_data["Tref_C"], TEMP_UNIT_CELSIUS));

    if(thermistor_ == nullptr)
    {
        formatError(err, MALLOC_FAILED);
        return;
    }

    nsamples_ = Registry::instance().config().get("sensor.average_len", DEFAULT_MOVING_AVERAGE_LEN);
}


// sense() - take a temperature reading from the temperature sensor, update the moving average, and return the
// moving-average value through <T>.  Return true on success, false on error.
//
bool TemperatureSensor::sense(Temperature& T, Error * const err)
{
    double voltage;

    if(!readRaw(voltage, err))
        return false;

    const Temperature sample = thermistor_->T(voltage / Registry::instance().adc().isource());

    // If this is the first sample, set the moving-average value to the sampled temperature in order to initialise it to
    // an approximate value.  If this is not the first sample, use the data to adjust the moving-average value.
    if(sampleTaken_)
    {
        // At least one sample has already been taken
        tempKelvin_ -= tempKelvin_ / nsamples_;
        tempKelvin_ += sample.K() / nsamples_;
    }
    else
    {
        // This is the first sample
        tempKelvin_ = sample.K();
        sampleTaken_ = true;
    }

    T.set(tempKelvin_, TEMP_UNIT_KELVIN);

    return true;
}

