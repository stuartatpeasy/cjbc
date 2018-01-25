/*
    session.cc: manages a single fermentation/conditioning session

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl

    TODO: mark session as complete at termination
*/

#include "include/application/session.h"
#include "include/framework/log.h"
#include "include/framework/registry.h"
#include "include/peripherals/effector.h"
#include "include/peripherals/tempsensor.h"
#include "include/util/validator.h"
#include <boost/algorithm/string.hpp>
#include <cstdlib>      // NULL
#include <ctime>        // ::time()

using boost::iequals;
using std::string;
namespace Validator = Util::Validator;


static const double
    DEFAULT_TEMP_DEADZONE       = 0.5;  // Temperature "dead zone", in deg C/K, within which effectors will not be
                                        // activated to modify the session temperature
static const time_t
    DEFAULT_SWITCH_INTERVAL_S   = 10;   // Default interval between effector updates, in seconds


Session::Session(const session_id_t id, Error * const err) noexcept
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
      type_(NONE),
      complete_(false)
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
       !session.bind(":id", id_, err))
        return;

    if(!session.step(err))
    {
        if(!err->code())
            formatError(err, NO_SUCH_SESSION, id);

        return;
    }

    gyle_id_ = session["gyle_id"].get<int>();
    profile_ = session["profile_id"].get<int>();
    start_ts_ = session["start_ts"].get<int>();

    SQLiteStmt gyle;
    if(!db.prepare("SELECT name FROM gyle WHERE id=:gyle_id", gyle, err) ||
       !gyle.bind(":gyle_id", gyle_id_, err))
        return;

    if(gyle.step(err))
        gyle_ = gyle["name"].get<string>();
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

    const string typeStr = profile["type"].get<string>();
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
        const time_t duration = session["duration_hours"].get<int>() * 3600;

        offset += duration;
        stages_.push_back(SessionStage_t(duration, session["temperature"].get<double>()));
    }
    
    if(stages_.empty())
    {
        formatError(err, NO_SUCH_PROFILE, profile_);
        return;
    }

    end_ts_ = offset;

    if(::time(NULL) >= end_ts_)
    {
        logWarning("Session %d is already complete", id_);
        complete_ = true;
    }

    deadZone_ = cfg.get("session.dead_zone", DEFAULT_TEMP_DEADZONE, Validator::gt0);
    effectorUpdateInterval_ = cfg.get("session.switch_interval_s", DEFAULT_SWITCH_INTERVAL_S, Validator::gt0);

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

    if(!((bool) t) || !tempSensorVessel_->inRange())
    {
        // Failed to sense temperature, or no sensor attached, or sensed temperature is out of the probe's range.
        // Deactivate effectors and return failure.
        effectorHeater_->activate(false);
        effectorCooler_->activate(false);
        tempControlState_ = UNKNOWN;

        return false;
    }

    const Temperature upperLimit = targetTemp() + Temperature(deadZone_, TEMP_UNIT_KELVIN),
                      lowerLimit = targetTemp() - Temperature(deadZone_, TEMP_UNIT_KELVIN);

    if(t > upperLimit)
    {
        // Temperature is above the dead zone surrounding the target temperature - activate cooling effectors.
        logDebug("Session %d (G%d): temp %.2fC is above dead zone (%.2fC); cooling",
                 id_, gyle_id_, t.C(), upperLimit.C());
        tempControlState_ = COOL;

        effectorHeater_->activate(false);   // Not ideal: error code not captured
        return effectorCooler_->activate(true);
    }
    else if(t < lowerLimit)
    {
        // Temperature is below the dead zone surrounding the target temperature - activate heating effectors.
        logDebug("Session %d (G%d): temp %.2fC is below dead zone (%.2fC); heating",
                 id_, gyle_id_, t.C(), lowerLimit.C());

        effectorCooler_->activate(false);   // Not ideal: error code not captured
        return effectorHeater_->activate(true);
    }
    else if((t >= targetTemp()) && effectorCooler_->state())
    {
        logDebug("Session %d (G%d): temp %.2fC is above target (%.2fC); cooling",
                 id_, gyle_id_, t.C(), targetTemp().C());

        return true;                        // No need to change effector state
    }
    else if((t <= targetTemp()) && effectorHeater_->state())
    {
        logDebug("Session %d (G%d): temp %.2fC is below target (%.2fC); heating",
                 id_, gyle_id_, t.C(), targetTemp().C());

        return true;                        // No need to change effector state
    }
    else
    {
        logDebug("Session %d (G%d): temp %.2fC is within target range %.2fC +/-%.2fC",
                 id_, gyle_id_, t.C(), targetTemp().C(), deadZone_);

        effectorCooler_->activate(false);   // Not ideal: error code not captured
        return effectorHeater_->activate(false);
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


// remainingTime() - return the number of seconds remaining in the session, or 0 if the session is inactive.
//
time_t Session::remainingTime() const noexcept
{
    const time_t now = ::time(NULL);

    if((now >= start_ts_) && (now < end_ts_))
        return end_ts_ - now;

    return 0;
}


// iterate() - entry-point for session management.  This method will be called in a loop by SessionManager::run()
//
bool Session::iterate(Error * const err) noexcept
{
    const time_t now = ::time(NULL);
    currentTemp();      // Always sense the current temperature: oversampling maintains the moving average

    if(now >= start_ts_)
    {
        if(now < end_ts_)
        {
            if((now - lastEffectorUpdate_) >= effectorUpdateInterval_)
            {
                updateEffectors();
                lastEffectorUpdate_ = now;
            }
        }
        else if(!complete_)
        {
            // Session has finished and needs to be finalised
            auto& db = Registry::instance().db();
            SQLiteStmt session;

            // Ensure that effectors are deactivated
            stop();

            // Mark session as complete in database
            if(!db.prepare("UPDATE session SET date_finish=NOW() WHERE id=:id", session, err) ||
               !session.bind(":id", id_, err) ||
               !session.step(err))
                return false;

            complete_ = true;
        }
    }

    return true;
}


// stop() - called when the session manager shuts down.
//
void Session::stop() noexcept
{
    logInfo("Session %d stopping", id_);

    // Switch off effectors before shutdown
    effectorHeater_->activate(false);
    effectorCooler_->activate(false);
}
