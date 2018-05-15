/*
    session.cc: manages a single fermentation/conditioning/serving session

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
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
    DEFAULT_TEMP_DEADZONE           = 0.5;  // Temperature "dead zone", in deg C/K, within which effectors will not be
                                            // activated to modify the session temperature
static const time_t
    DEFAULT_EFF_UPDATE_INTERVAL_S   = 1;    // Default interval between effector updates, in seconds


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

    // As a precaution, deactivate effectors
    effectorHeater_->activate(false);
    effectorCooler_->activate(false);

    // Read basic session information
    auto& cfg = Registry::instance().config();
    auto& db = Registry::instance().db();
    SQLiteStmt session;

    // FIXME: if a value is set for date_finish, and that value is <=CURRENT_TIMESTAMP, log a message and set
    // complete_ = true.
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
    else if(iequals(typeStr, "serve"))
        type_ = SERVE;
    else
    {
        formatError(err, BAD_PROFILE_TYPE, profile_, typeStr.c_str());
        return;
    }

    // Read session temperature-stage information
    if(!db.prepare("SELECT stage, duration_hours, temperature FROM profilestage "
                   "WHERE profile_id=:id ORDER BY stage", session, err) ||
       !session.bind(":id", profile_, err))
        return;

    time_t offset = start_ts_;
    while(session.step(err))
    {
        SessionStage_t stage;
        stage.temperature = session["temperature"].get<double>();

        if(session["duration_hours"].isNull())
        {
            // A NULL value in the "duration_hours" field indicates that the record contains a temperature at which
            // the session should be held indefinitely following completion of the profile, i.e. the temperature-
            // maintenance stage.  This temperature will be held "forever", i.e. until the operator intervenes to tell
            // the control system that the stage should be terminated.  "Serving" sessions typically have only a single
            // stage, with a "forever" temperature.
            stage.duration = 0;
            stage.forever = true;
            stages_.push_back(stage);

            break;      // This must be the last stage in the profile - ignore any subsequent stages.
        }
        else
        {
            stage.duration = session["duration_hours"].get<int>() * 3600;
            stage.forever = false;

            // For fermentation and conditioning sessions, append each temperature step, and corresponding duration, into
            // stages_.
            offset += stage.duration;
            stages_.push_back(stage);
        }
    }
    
    if(stages_.empty())
    {
        formatError(err, NO_SUCH_PROFILE, profile_);
        return;
    }

    end_ts_ = offset;

    deadZone_ = cfg.get("session.dead_zone", DEFAULT_TEMP_DEADZONE, Validator::gt0);
    effectorUpdateInterval_ = cfg.get("session.effector_update_interval_s", DEFAULT_EFF_UPDATE_INTERVAL_S,
                                        Validator::gt0);
}


// targetTemp() - return the current target temperature for this session.
//
Temperature Session::targetTemp() noexcept
{
    const time_t now = ::time(NULL);

    if(now >= start_ts_)
    {
        time_t offset = start_ts_;
        for(auto stage : stages_)
        {
            if(stage.forever || ((offset + stage.duration) > now))
                return Temperature(stage.temperature, TEMP_UNIT_CELSIUS);

            offset += stage.duration;
        }
    }

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

    const Temperature t = tempSensorVessel_->sense(err);

    if(!((bool) t) || !tempSensorVessel_->inRange())
    {
        // Failed to sense temperature, or no sensor attached, or sensed temperature is out of the probe's range.
        // Deactivate effectors and return failure.
        effectorHeater_->activate(false);
        effectorCooler_->activate(false);
        tempControlState_ = UNKNOWN;

        return false;
    }

    const Temperature target = targetTemp(),
                      upperLimit = target + Temperature(deadZone_, TEMP_UNIT_KELVIN),
                      lowerLimit = target - Temperature(deadZone_, TEMP_UNIT_KELVIN);

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
        tempControlState_ = HEAT;

        effectorCooler_->activate(false);   // Not ideal: error code not captured
        return effectorHeater_->activate(true);
    }
    else if((t >= target) && effectorCooler_->state())
    {
        logDebug("Session %d (G%d): temp %.2fC is above target (%.2fC); cooling",
                 id_, gyle_id_, t.C(), target.C());
        tempControlState_ = COOL;

        return true;                        // No need to change effector state
    }
    else if((t <= target) && effectorHeater_->state())
    {
        logDebug("Session %d (G%d): temp %.2fC is below target (%.2fC); heating",
                 id_, gyle_id_, t.C(), target.C());
        tempControlState_ = HEAT;

        return true;                        // No need to change effector state
    }
    else
    {
        logDebug("Session %d (G%d): temp %.2fC is within target range %.2fC +/-%.2fC",
                 id_, gyle_id_, t.C(), target.C(), deadZone_);
        tempControlState_ = HOLD;

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
    return !complete_ && (::time(NULL) >= start_ts_);
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
    (void) err;         // Suppress arg-not-used warning; arg is likely to be used in future.

    const time_t now = ::time(NULL);

    currentTemp();      // Always sense the current temperature: oversampling maintains the moving average

    if((now - lastEffectorUpdate_) >= effectorUpdateInterval_)
    {
        if(now >= start_ts_)
        {
            updateEffectors();
            lastEffectorUpdate_ = now;
        }
        else
            deactivateEffectors();
    }

    return true;
}


// markComplete() - update the session's database entry, setting its completion timestamp (session.date_finish) equal to
// the current timestamp, and set the "complete_" member var to true.  Returns true on success, false otherwise.
//
bool Session::markComplete(Error * const err) noexcept
{
    auto& db = Registry::instance().db();
    SQLiteStmt session;

    if(!deactivateEffectors())
        logWarning("Session %d: markComplete(): failed to deactivate effectors; marking session complete anyway.", id_);

    // Mark session as complete in database
    if(!db.prepare("UPDATE session SET date_finish=NOW() WHERE id=:id", session, err) ||
       !session.bind(":id", id_, err) ||
       !session.step(err))
        return false;

    complete_ = true;

    return true;
}


// deactivateEffectors() - switch off heating/cooling effectors.  Returns true if both operations were successful;
// false otherwise.
//
bool Session::deactivateEffectors() noexcept
{
    return effectorHeater_->activate(false) && effectorCooler_->activate(false);
}


// stop() - called when the session manager shuts down.
//
void Session::stop() noexcept
{
    logInfo("Session %d stopping", id_);

    deactivateEffectors();
}

