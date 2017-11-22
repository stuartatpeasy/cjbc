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
#include <boost/algorithm/string.hpp>
#include <cstdlib>      // NULL
#include <ctime>        // ::time()

using boost::iequals;
using std::string;


static const double
    DEFAULT_TEMP_DEADZONE               = 0.5;  // Temperature "dead zone", in deg C/K, within which effectors will not
                                                // be activated to modify the session temperature
static const time_t
    DEFAULT_EFFECTOR_UPDATE_INTERVAL_S  = 10;   // Default interval between effector updates, in seconds


Session::Session(const int id, Error * const err) noexcept
    : id_(id),
      gyle_id_(0),
      profile_(0),
      start_ts_(0),
      end_ts_(0),
      deadZone_(0.0),
      effectorUpdateInterval_(0),
      lastEffectorUpdate_(0),
      tempSensorVessel_(TempSensor::getSessionVesselTempSensor(id, err)),
      effectorHeater_(Effector::getSessionHeater(id, err)),
      effectorCooler_(Effector::getSessionCooler(id, err)),
      tempControlState_(HOLD),
      type_(NONE)
{
    if(err->code())
        return;         // Stop if initialisation of any member variable failed

    // Read basic session information
    auto& cfg = Registry::instance().config();
    auto& db = Registry::instance().db();
    SQLiteStmt session;

    if(!db.prepare("SELECT gyle_id, profile_id, CAST((JULIANDAY(date_start) - 2440587.5) * 86400.0 AS INT) AS start_ts "
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

    gyle_id_ = session["gyle_id"];
    profile_ = session["profile_id"];
    start_ts_ = session["start_ts"];

    SQLiteStmt gyle;
    if(!db.prepare("SELECT name FROM gyle WHERE id=:gyle_id", gyle, err) ||
       !gyle.bind(":gyle_id", gyle_id_, err))
        return;

    if(gyle.step(err))
        gyle_ = gyle["name"].asString();
    else
    {
        if(err->code())
            return;

        gyle_ = "<unknown gyle>";
    }

    // Read profile information
    SQLiteStmt profile;
    if(!db.prepare("SELECT type FROM profile WHERE id=:profileId", profile, err) ||
       !profile.bind(":profileId", profile_, err))
        return;

    if(!profile.step(err))
    {
        if(!err->code())
            formatError(err, NO_SUCH_PROFILE, profile_);

        return;
    }

    const string typeStr = profile["type"].asString();
    if(iequals(typeStr, "ferment"))
        type_ = FERMENT;
    else if(iequals(typeStr, "condition"))
        type_ = CONDITION;
    else
    {
        formatError(err, BAD_PROFILE_TYPE, profile_, typeStr.c_str());
        return;
    }

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
    
    if(stages_.empty())
    {
        formatError(err, NO_SUCH_PROFILE, profile_);
        return;
    }

    end_ts_ = offset;

    deadZone_ = cfg.get("session.dead_zone", DEFAULT_TEMP_DEADZONE);
    effectorUpdateInterval_ = cfg.get("session.effector_update_interval_s", DEFAULT_EFFECTOR_UPDATE_INTERVAL_S);

    effectorHeater_->activate(false);
    effectorCooler_->activate(false);
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


// vesselTempSensorInRange() - returns bool indicating whether the FV/CV temp sensor reading is within the sensor's
// valid measurement range.
//
bool Session::vesselTempSensorInRange() const noexcept
{
    return tempSensorVessel_->inRange();
}


// updateEffectors() - switch on (or off) the session's heater/cooler as required, in order to steer the session
// temperature towards the target temperature.
//
bool Session::updateEffectors(Error * const err) noexcept
{
    if(!isActive())
    {
        // Session is inactive; ensure that its effectors are deactivated.
        effectorHeater_->activate(false);
        effectorCooler_->activate(false);
        tempControlState_ = HOLD;

        return true;
    }

    Temperature t = tempSensorVessel_->sense(err);
    if(!t)
    {
        // Failed to sense temperature, or no sensor attached.  Deactivate effectors and return failure.
        effectorHeater_->activate(false);
        effectorCooler_->activate(false);
        tempControlState_ = HOLD;

        return false;
    }

    const double diff = t.diff(targetTemp(), TEMP_UNIT_CELSIUS);

    if(::fabs(diff) < deadZone_)
    {
        // Temperature is within dead zone - deactivate all effectors
        logDebug("Session %d: temperature is within range", id_);
        tempControlState_ = HOLD;

        effectorHeater_->activate(false);   // Not ideal: error code not captured
        return effectorCooler_->activate(false, err);
    }
    else if(diff > 0.0)
    {
        // Temperature is too high - activate cooling effectors
        logDebug("Session %d: temperature is too high; cooling", id_);
        tempControlState_ = COOL;

        effectorHeater_->activate(false);   // Not ideal: error code not captured
        return effectorCooler_->activate(true, err);
    }
    else if(diff < 0.0)
    {
        // Temperature is too low - activate heating effectors
        logDebug("Session %d: temperature is too low; heating", id_);
        tempControlState_ = HEAT;

        effectorCooler_->activate(false);   // Not ideal: error code not captured
        return effectorHeater_->activate(true, err);
    }

    return true;
}


// isNotStartedYet() - return bool indicating whether the current session has not started yet, i.e. it is scheduled to
// start in the future.
//
bool Session::isNotStartedYet() const noexcept
{
    return ::time(NULL) < start_ts_;
}


// isActive() - return bool indicating whether the current session is "active", i.e. the current time is between the
// session's start time and its end time.
//
bool Session::isActive() const noexcept
{
    const time_t now = ::time(NULL);

    return (now >= start_ts_) && (now < end_ts_);
}


// isComplete() - return bool indicating whether the current session has finished.
//
bool Session::isComplete() const noexcept
{
    return ::time(NULL) >= end_ts_;
}


// remainingTime() - return the number of seconds remaining in the session, or 0 if the session is inactive.
//
time_t Session::remainingTime() const noexcept
{
    const time_t now = ::time(NULL);

    if((now >= start_ts_) && (now < end_ts_))
        return end_ts_ - now;

    return 0;
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

