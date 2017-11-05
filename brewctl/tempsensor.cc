/*
    tempsensor.cc: models a temperature sensor (i.e. a thermistor) attached to an ADC channel.  Assumes that there
    is a constant current flowing through the sensor and developing a voltage across it.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "tempsensor.h"
#include "log.h"
#include "nulltempsensor.h"
#include "registry.h"
#include "sqlitestmt.h"
#include <string>

using std::string;


#define DEFAULT_MOVING_AVERAGE_LEN      (1000)  // Default length of moving average for sensor readings


TempSensor::TempSensor(const int thermistor_id, const int channel, Error * const err) noexcept
    : channel_(channel), thermistor_(nullptr), sampleTaken_(false)
{
    // Initialise: read sensor data from the database
    SQLite& db = Registry::instance().db();
    SQLiteStmt thermistor_data;

    if(!db.prepare("SELECT name, type, Tref_C, Rref, beta, range_min, range_max "
                   "FROM thermistor WHERE id=:id", thermistor_data, err)
       || !thermistor_data.bind(":id", thermistor_id, err))
        return;

    if(!thermistor_data.step(err))
    {
        if(err->code())
            return;

        formatError(err, NO_SUCH_THERMISTOR, thermistor_id);
        return;
    }

    if(thermistor_data["type"].asString() != "NTC")
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

    rangeMin_ = thermistor_data["range_min"];
    rangeMax_ = thermistor_data["range_max"];

    nsamples_ = Registry::instance().config().get("sensor.average_len", DEFAULT_MOVING_AVERAGE_LEN);
}


// move ctor
//
TempSensor::TempSensor(TempSensor&& rhs) noexcept
{
    move(rhs);
}


TempSensor::~TempSensor() noexcept
{
}


// move-assignment operator
//
TempSensor& TempSensor::operator=(TempSensor&& rhs) noexcept
{
    move(rhs);
    return *this;
}


// move() - helper method used by move ctor and move-assignment operator
//
void TempSensor::move(TempSensor& rhs) noexcept
{
    channel_ = rhs.channel_;
    thermistor_ = rhs.thermistor_;
    nsamples_ = rhs.nsamples_;
    Idrive_ = rhs.Idrive_;
    sampleTaken_ = rhs.sampleTaken_;
    tempKelvin_ = rhs.tempKelvin_;
    name_ = rhs.name_;

    rhs.channel_ = -1;
    rhs.thermistor_ = nullptr;
    rhs.nsamples_ = 0;
    rhs.Idrive_ = 0.0;
    rhs.sampleTaken_ = false;
    rhs.tempKelvin_ = 0.0;
    rhs.name_ = "";
}


// sense() - take a temperature reading from the temperature sensor, update the moving average, and return the
// moving-average value through <T>.  Return the current moving-average temperature value on success, or a temperature
// value representing absolute zero on error.
//
Temperature TempSensor::sense(Error * const err) noexcept
{
    const double voltage = readRaw(err);
    if(voltage < 0.0)
        return Temperature();       

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

    return Temperature(tempKelvin_, TEMP_UNIT_KELVIN);
}


// inRange() - return bool indicating whether the current sampled temperature is within the sensing range of this
// object's thermistor.
//
bool TempSensor::inRange() const noexcept
{
    return (tempKelvin_ >= rangeMin_) && (tempKelvin_ <= rangeMax_);
}


// getTempSensor() - factory for TempSensor objects.  Returns a TempSensor object representing the sensor specified by
// <session_id> and <role>.  If no such sensor exists in the database, a NullTempSensor() object is returned; this acts
// as a "null" sensor, always safely returning a temperature of absolute zero.  If the database lookup fails, nullptr is
// returned.
//
TempSensor* TempSensor::getTempSensor(const int sessionId, const string& role, Error * const err) noexcept
{
    auto& db = Registry::instance().db();
    SQLiteStmt tempSensor;

    if(!db.prepare("SELECT channel, thermistor_id FROM temperaturesensor "
                   "WHERE role=:role AND session_id=:session_id", tempSensor, err) ||
       !tempSensor.bind(":role", role) ||
       !tempSensor.bind(":session_id", sessionId))
        return nullptr;

    if(!tempSensor.step(err))
        return err->code() ? nullptr : new NullTempSensor();

    return new TempSensor(tempSensor["thermistor_id"], tempSensor["channel"], err);
}


// getSessionVesselTempSensor() - obtain a TempSensor object representing the sensor measuring the vessel temperature in
// the session specified by <sessionId>.
//
TempSensor* TempSensor::getSessionVesselTempSensor(const int sessionId, Error * const err) noexcept
{
    return getTempSensor(sessionId, "vessel", err);
}


// getAmbientTempSensor() - obtain a TempSensor object representing the sensor measuring brewhouse ambient temperature,
// if such a sensor is present.
//
TempSensor* TempSensor::getAmbientTempSensor(Error * const err) noexcept
{
    return getTempSensor(0, "ambient", err);
}


// readRaw() - read the raw voltage on our ADC channel; return it as a double.
//
double TempSensor::readRaw(Error * const err) noexcept
{
    return Registry::instance().adc().read(channel_, err);
}


