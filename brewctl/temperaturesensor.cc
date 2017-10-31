/*
    temperaturesensor.h: models a temperature sensor (i.e. a thermistor) attached to an ADC channel.  Assumes that there
    is a constance current flowing through the sensor and developing a voltage across it.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "temperaturesensor.h"
#include "registry.h"
#include "sqlitestmt.h"


TemperatureSensor::TemperatureSensor(const int thermistor_id, const int channel, Error * const err)
    : Sensor(channel), thermistor_id_(thermistor_id), thermistor_(nullptr), tempKelvin_(0.0), sampleTaken_(false)
{
    // Initialise: read sensor data from the database
    SQLite& db = Registry::instance().db();
    SQLiteStmt thermistor_data;

    if(!db.prepare("SELECT name, type, Tref_C, Rref, beta, range_min, range_max "
                   "FROM thermistor WHERE id=:id", thermistor_data, err)
       || !thermistor_data.step(err))
        return;

    if(thermistor_data.column(1)->asString() != "thermistor")
    {
        formatError(err, SENSOR_INVALID_TYPE, thermistor_data.column(1)->asCString());
        return;
    }

    name_ = thermistor_data.column(0)->asString();
    
    thermistor_ = new Thermistor(
        thermistor_data.column(4)->asDouble(),                                  // beta
        thermistor_data.column(3)->asDouble(),                                  // R0 (Rref)
        Temperature(thermistor_data.column(2)->asDouble(), TEMP_UNIT_CELSIUS)); // T0

    if(thermistor_ == nullptr)
    {
        formatError(err, MALLOC_FAILED);
        return;
    }

    nsamples_ = 1000;       // FIXME read from config
    Idrive_ = 0.000147;     // FIXME read from config

}


// sense() - take a temperature reading from the temperature sensor, update the moving average, and return the
// moving-average value through <T>.  Return true on success, false on error.
//
bool TemperatureSensor::sense(Temperature& T, Error * const err)
{
    double voltage;

    if(!readRaw(voltage, err))
        return false;

    const Temperature sample = thermistor_->T(voltage / Idrive_);

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

