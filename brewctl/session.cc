/*
    session.cc: manages a single fermentation/conditioning session

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "session.h"
#include "log.h"
#include "registry.h"
#include <cstdlib>      // NULL


#define DEFAULT_TEMP_DEADZONE   (0.5)       // Temperature "dead zone", in deg C/K, within which effectors will not be
                                            // activated to modify the session temperature


Session::Session(const int id, Error * const err)
    : id_(id), start_ts_(0)
{
    // Read basic session information
    auto& db = Registry::instance().db();
    SQLiteStmt session;

    if(!db.prepare("SELECT gyle, profile_id, CAST((JULIANDAY(date_start) - 2440587.5) * 86400.0 AS INT) AS start_ts "
                   "FROM session "
                   "WHERE id=:id", session, err) ||
       !session.bind(":id", id, err) ||
       !session.step(err))
        return;

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

    // Read session sensor configuration
    SQLiteStmt sensor;
    if(!db.prepare("SELECT channel, thermistor_id FROM temperaturesensor "
                   "WHERE role='vessel' AND session_id=:id", sensor, err) ||
       !sensor.bind(":id", id, err))
        return;

    if(sensor.step(err))
    {
        if(!err->code())
            logWarning("Session %d has no vessel temperature sensor");
        else
            return;
    }

    tempSensorVessel_ = new TemperatureSensor(sensor["channel"], sensor["thermistor_id"]);
    if(tempSensorVessel_ == nullptr)
    {
        formatError(err, MALLOC_FAILED);
        return;
    }

    // Read session effector configuration
    effectorHeater_ = Effector::getSessionHeater(id_, err);
    if(effectorHeater_ == nullptr)
        return;

    effectorCooler_ = Effector::getSessionCooler(id_, err);
    if(effectorCooler_ == nullptr)
        return;

    deadZone_ = Registry::instance().config().get("session.dead_zone", DEFAULT_TEMP_DEADZONE);
}


// targetTemp() - return the current target temperature for this session.
//
Temperature Session::targetTemp()
{
    const time_t now = ::time(NULL);
    time_t offset = start_ts_;

    for(auto it = stages_.begin(); it != stages_.end(); ++it)
        if((offset + it->first) > now)
            return Temperature(it->second, TEMP_UNIT_CELSIUS);

    // Session is not active
    return Temperature(0, TEMP_UNIT_KELVIN);
}


bool Session::updateEffectors(Error * const err)
{
    if(!isActive())
        return true;

    // Sense current temperature
    if(tempSensorVessel_ == nullptr)
        return false;                   // No temperature sensor available for this session

    Temperature t;

    if(!tempSensorVessel_->sense(t, err))
        return false;

    const double diff = t.diff(targetTemp(), TEMP_UNIT_CELSIUS);
    bool ok = true;

    if(::fabs(diff) < deadZone_)
    {
        // Temperature is within dead zone - deactivate all effectors
        if(effectorHeater_ != nullptr)
            ok &= effectorHeater_->activate(false, err);

        if(effectorCooler_ != nullptr)
            ok &= effectorCooler_->activate(false, err);
    }
    else if(diff > 0.0)
    {
        // Temperature is too high - activate cooling effectors
        if(effectorCooler_ != nullptr)
            ok &= effectorCooler_->activate(true, err);
    }
    else if(diff < 0.0)
    {
        // Temperature is too low - activate heating effectors
        if(effectorCooler_ != nullptr)
            ok &= effectorCooler_->activate(true, err);
    }

    return ok;
}


// isActive() - return bool indicating whether the current session is "active", i.e. the current time is between the
// session's start time and its end time.
//
bool Session::isActive() const
{
    const time_t now = ::time(NULL);

    return (now >= start_ts_) && (now < end_ts_);
}

