/*
    session.cc: manages a single fermentation/conditioning session

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "session.h"
#include "registry.h"
#include <cstdlib>      // NULL

#include <iostream>     // FIXME remove
using namespace std;    // FIXME remove

Session::Session(const int id, Error * const err)
    : id_(id), start_ts_(0)
{
    // Read basic session information
    SQLite& db = Registry::instance().db();
    SQLiteStmt session;

    if(!db.prepare("SELECT gyle, profile_id, "
                       "CAST((JULIANDAY(date_start) - 2440587.5) * 86400.0 AS INT) AS start_ts "
                   "FROM session "
                   "WHERE id=:id", session, err) ||
       !session.bind(":id", id, err) ||
       !session.step(err))
        return;

    gyle_ = session.column(0)->asString();
    profile_ = session.column(1)->asInt();
    start_ts_ = session.column(2)->asInt();

    // Read session temperature-stage information
    if(!db.prepare("SELECT stage, duration_hours, temperature FROM profilestage "
                   "WHERE profile_id=:id", session, err) ||
       !session.bind(":id", profile_, err))
        return;

    time_t offset = start_ts_;
    while(session.step(err))
    {
        const time_t duration = session.column(1)->asDouble() * 3600;

        offset +=  duration;
        stages_.push_back(SessionStage_t(duration, session.column(2)->asDouble()));

        cout << "Session stage: duration " << duration / 3600 << "h, temp " << session.column(2)->asDouble() << endl;
    }

    end_ts_ = offset;

    if(isActive())
        cout << "Session is active; current temp should be " << targetTemp().C() << "deg C" << endl;
    else
        cout << "Session is not active" << endl;

    // Read session sensor configuration
    SQLiteStmt sensor;
    if(!db.prepare("SELECT sensortype_id, channel FROM sessionsensor WHERE session_id=:id",
                   sensor, err) ||
       !sensor.bind(":id", id, err))
        return;

    while(sensor.step(err))
    {
        // FIXME hmm
//        createSensor(db_, adc_, sensor.column(0)->asInt(), sensor.column(1)->asInt(), err);
    }

    if(err->code())
        return;
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

/*
bool Session::updateEffectors()
{
    if(!isActive())
        return true;

    // Sense current temperature
}
*/

// isActive() - return bool indicating whether the current session is "active", i.e. the current
// time is between the session's start time and its end time.
//
bool Session::isActive() const
{
    const time_t now = ::time(NULL);

    return (now >= start_ts_) && (now < end_ts_);
}

