/*
    tempsensor.cc: models a temperature sensor (i.e. a thermistor) attached to an ADC channel.  Assumes that there
    is a constant current flowing through the sensor and developing a voltage across it.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "tempsensor.h"
#include "log.h"
#include "registry.h"
#include "sqlitestmt.h"
#include <string>

using std::string;


static const int 
    DEFAULT_MOVING_AVERAGE_LEN  = 1000,     // Default length of moving average for sensor readings
    DEFAULT_LOG_INTERVAL_S      = 10;       // Default interval between temp-sensor log writes, in seconds


TempSensor::TempSensor(const int thermistor_id, const int channel, Error * const err) noexcept
    : DefaultTempSensor(channel, "TempSensor"),
      thermistor_(nullptr),
      avglen_(0),
      nsamples_(0),
      currentTemp_(0.0, TEMP_UNIT_CELSIUS),
      rangeMin_(0.0, TEMP_UNIT_KELVIN),
      rangeMax_(1000.0, TEMP_UNIT_KELVIN),
      lastLogWriteTime_(0),
      logInterval_(0)
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

    rangeMin_.set(thermistor_data["range_min"], TEMP_UNIT_CELSIUS);
    rangeMax_.set(thermistor_data["range_max"], TEMP_UNIT_CELSIUS);

    auto& config = Registry::instance().config();

    avglen_ = config.get("sensor.average_len", DEFAULT_MOVING_AVERAGE_LEN);
    if(avglen_ < 1)
    {
        logWarning("Invalid value (%d) for sensor.average_len; using sensor.average_len=1 instead", avglen_);
        avglen_ = 1;
    }

    logInterval_ = config.get("sensor.log_interval_s", DEFAULT_LOG_INTERVAL_S);
    if(logInterval_ < 0)
    {
        logWarning("Invalid value (%d) for sensor.log_interval_s; disabling temperature logging for channel %d",
                    logInterval_, channel_);
        logInterval_ = 0;
    }
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
    channel_            = rhs.channel_;
    thermistor_         = rhs.thermistor_;
    avglen_             = rhs.avglen_;
    nsamples_           = rhs.nsamples_;
    Idrive_             = rhs.Idrive_;
    name_               = rhs.name_;
    currentTemp_        = rhs.currentTemp_;
    rangeMin_           = rhs.rangeMin_;
    rangeMax_           = rhs.rangeMax_;
    lastLogWriteTime_   = rhs.lastLogWriteTime_;
    logInterval_        = rhs.logInterval_;

    rhs.channel_            = -1;
    rhs.thermistor_         = nullptr;
    rhs.avglen_             = 0;
    rhs.Idrive_             = 0.0;
    rhs.nsamples_           = 0;
    rhs.name_               = "";
    rhs.currentTemp_        = Temperature(0.0, TEMP_UNIT_KELVIN);
    rhs.rangeMin_           = Temperature(0.0, TEMP_UNIT_KELVIN);
    rhs.rangeMax_           = Temperature(0.0, TEMP_UNIT_KELVIN);
    rhs.lastLogWriteTime_   = 0;
    rhs.logInterval_        = 0;
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
    if(nsamples_ < avglen_)
        ++nsamples_;

    const double tempCelsius = currentTemp_.C() + ((sample.C() - currentTemp_.C()) / nsamples_);

    currentTemp_.set(tempCelsius, TEMP_UNIT_CELSIUS);
    writeTempLog();

    return currentTemp_;
}


// inRange() - return bool indicating whether the current sampled temperature is within the sensing range of this
// object's thermistor.
//
bool TempSensor::inRange() const noexcept
{
    return (currentTemp_ >= rangeMin_) && (currentTemp_ <= rangeMax_);
}


// getTempSensor() - factory for TempSensor objects.  Returns a TempSensor object representing the sensor specified by
// <session_id> and <role>.  If no such sensor exists in the database, a NullTempSensor() object is returned; this acts
// as a "null" sensor, always safely returning a temperature of absolute zero.  If any database lookup step fails,
// a NullTempSensor object is returned and <err> is set accordingly.
//
DefaultTempSensor_uptr_t TempSensor::getTempSensor(const int sessionId, const string& role, Error * const err)
    noexcept
{
    SQLiteStmt tempSensor;
    DefaultTempSensor* ret;

    if(!Registry::instance().db().prepare("SELECT channel, thermistor_id FROM temperaturesensor "
                                          "WHERE role=:role AND session_id=:session_id", tempSensor, err)
       || !tempSensor.bind(":role", role, err)
       || !tempSensor.bind(":session_id", sessionId, err)
       || !tempSensor.step(err))
    {
        logInfo("Session %d: no temperature sensor found for role '%s'", sessionId, role.c_str());
        ret = new DefaultTempSensor();
    }
    else
    {
        logDebug("returning TempSensor for role %s on channel %d", role.c_str(), tempSensor["channel"].asInt());
        ret = new TempSensor(tempSensor["thermistor_id"], tempSensor["channel"], err);
    }

    if(ret == nullptr)
        formatError(err, MALLOC_FAILED);

    return DefaultTempSensor_uptr_t(ret);
}


// getSessionVesselTempSensor() - obtain a TempSensor object representing the sensor measuring the vessel temperature in
// the session specified by <sessionId>.
//
DefaultTempSensor_uptr_t TempSensor::getSessionVesselTempSensor(const int sessionId, Error * const err) noexcept
{
    return getTempSensor(sessionId, "vessel", err);
}


// getAmbientTempSensor() - obtain a TempSensor object representing the sensor measuring brewhouse ambient temperature,
// if such a sensor is present.
//
DefaultTempSensor_uptr_t TempSensor::getAmbientTempSensor(Error * const err) noexcept
{
    return getTempSensor(0, "ambient", err);
}


// readRaw() - read the raw voltage on our ADC channel; return it as a double.
//
double TempSensor::readRaw(Error * const err) noexcept
{
    return Registry::instance().adc().read(channel_, err);
}


// writeTempLog() - if enough time has passed since the last temperature reading was written to the temperature log,
// write the current reading to the log.  Swallow any errors that occur.
//
void TempSensor::writeTempLog()
{
    const time_t now = ::time(NULL);

    if(logInterval_ && ((now - lastLogWriteTime_) > logInterval_))
    {
        SQLiteStmt logStmt;

        Registry::instance().db().prepare("INSERT INTO temperature(date_create, sensor_id, temperature) "
                                          "VALUES(CURRENT_TIMESTAMP, :sensor_id, :temperature)", logStmt)
        && logStmt.bind(":sensor_id", channel_)
        && logStmt.bind(":temperature", currentTemp_.C())
        && logStmt.step();

        lastLogWriteTime_ = now;
    }
}

