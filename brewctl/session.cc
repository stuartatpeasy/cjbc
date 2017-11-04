/*
    session.cc: manages a single fermentation/conditioning session

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "session.h"
#include "effector.h"
#include "log.h"
#include "registry.h"
#include "tempsensor.h"
#include <cstdlib>      // NULL
#include <ctime>        // ::time()


#define DEFAULT_TEMP_DEADZONE               (0.5)   // Temperature "dead zone", in deg C/K, within which effectors will
                                                    // not be activated to modify the session temperature
#define DEFAULT_EFFECTOR_UPDATE_INTERVAL_S  (10)    // Default interval between effector updates, in seconds


Session::Session(const int id, Error * const err) noexcept
    : id_(id), start_ts_(0), tempSensorVessel_(nullptr), effectorHeater_(nullptr), effectorCooler_(nullptr),
      lastEffectorUpdate_(0)
{
    // Read basic session information
    auto& cfg = Registry::instance().config();
    auto& db = Registry::instance().db();
    SQLiteStmt session;

    if(!db.prepare("SELECT gyle, profile_id, CAST((JULIANDAY(date_start) - 2440587.5) * 86400.0 AS INT) AS start_ts "
                   "FROM session "
                   "WHERE id=:id", session, err) ||
       !session.bind(":id", id, err))
        return;

    if(!session.step(err))
    {
        if(!err->code())
            formatError(err, NO_SUCH_SESSION, id);

        return;
    }

    gyle_ = session["gyle"].asString();
    profile_ = session["profile_id"];
    start_ts_ = session["start_ts"];

    // Read session temperature-stage information
    if(!db.prepare("SELECT stage, duration_hours, temperature FROM profilestage "
                   "WHERE profile_id=:id", session, err) ||
       !session.bind(":id", profile_, err))
        return;

    time_t offset = start_ts_;
    while(session.step(err))
    {
        const time_t duration = session["duration_hours"].asInt() * 3600;

        offset += duration;
        stages_.push_back(SessionStage_t(duration, session["temperature"]));
    }

    end_ts_ = offset;

    // Create session sensor and effector objects
    if(((tempSensorVessel_ = TempSensor::getSessionVesselTempSensor(id, err)) == nullptr) ||
       ((effectorHeater_ = Effector::getSessionHeater(id_, err)) == nullptr) ||
       ((effectorCooler_ = Effector::getSessionCooler(id_, err)) == nullptr))
        return;

    deadZone_ = cfg.get("session.dead_zone", DEFAULT_TEMP_DEADZONE);
    effectorUpdateInterval_ = cfg.get("session.effector_update_interval_s", DEFAULT_EFFECTOR_UPDATE_INTERVAL_S);
}


// dtor - free allocated resources
//
Session::~Session() noexcept
{
    if(tempSensorVessel_ != nullptr)
        delete tempSensorVessel_;

    if(effectorHeater_ != nullptr)
        delete effectorHeater_;

    if(effectorCooler_ != nullptr)
        delete effectorCooler_;
}


// targetTemp() - return the current target temperature for this session.
//
Temperature Session::targetTemp() noexcept
{
    const time_t now = ::time(NULL);
    time_t offset = start_ts_;

    for(auto it : stages_)
        if((offset + it.first) > now)
            return Temperature(it.second, TEMP_UNIT_CELSIUS);

    // Session is not active; return absolute zero.
    return Temperature();
}


// currentTemp() - return the current temperature of this session's vessel.  If there is no temperature sensor attached
// to the vessel, return a Temperature object indicating absolute zero.
//
Temperature Session::currentTemp() noexcept
{
    return tempSensorVessel_->sense();
}


// updateEffectors() - switch on (or off) the session's heater/cooler as required, in order to steer the session
// temperature towards the target temperature.
//
bool Session::updateEffectors(Error * const err) noexcept
{
    if(!isActive())
        return true;

    Temperature t = tempSensorVessel_->sense(err);
    if(!t)
        return false;                   // Failed to sense temperature, or no sensor attached

    const double diff = t.diff(targetTemp(), TEMP_UNIT_CELSIUS);

    if(::fabs(diff) < deadZone_)
    {
        // Temperature is within dead zone - deactivate all effectors
        logDebug("Session %d: temperature is within range", id_);
        return effectorHeater_->activate(false, err) && effectorCooler_->activate(false, err);
    }
    else if(diff > 0.0)
    {
        // Temperature is too high - activate cooling effectors
        logDebug("Session %d: temperature is too high; cooling", id_);
        return effectorCooler_->activate(true, err);
    }
    else if(diff < 0.0)
    {
        // Temperature is too low - activate heating effectors
        logDebug("Session %d: temperature is too low; heating", id_);
        return effectorCooler_->activate(true, err);
    }

    return true;
}


// isActive() - return bool indicating whether the current session is "active", i.e. the current time is between the
// session's start time and its end time.
//
bool Session::isActive() const noexcept
{
    const time_t now = ::time(NULL);

    return (now >= start_ts_) && (now < end_ts_);
}


// main() - entry-point for session management.  This method will be called in a loop by SessionManager::run()
//
void Session::main() noexcept
{
    const time_t now = ::time(NULL);
    currentTemp();      // Always sense the current temperature: oversampling maintains the moving average

    if((now - lastEffectorUpdate_) >= effectorUpdateInterval_)
    {
        updateEffectors();
        lastEffectorUpdate_ = now;
    }
}

