/*
    sessionmanager.cc: manages fermentation / conditioning sessions

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "include/application/sessionmanager.h"
#include "include/application/temperature.h"
#include "include/framework/log.h"
#include "include/peripherals/tempsensor.h"
#include <thread>

extern "C"
{
#include <unistd.h>         // ::usleep()
}

using std::thread;


// ctor - trivial initialisation of members
//
SessionManager::SessionManager() noexcept
    : Thread(),
      display_(*this)
{
}


// dtor - free sessions
//
SessionManager::~SessionManager() noexcept
{
    for(auto it : sessions_)
        delete it;
}


// init() - read active sessions from the database and build a vector of them in <sessions_>.
//
bool SessionManager::init(Error * const err) noexcept
{
    SQLite& db = Registry::instance().db();
    SQLiteStmt sessions;

    tempSensorAmbient_ = TempSensor::getAmbientTempSensor(err);
    if(err->code())
        return false;

    if(!db.prepare("SELECT id "
                   "FROM session "
                   "WHERE date_start<=CURRENT_TIMESTAMP AND date_finish IS NULL "
                   "ORDER BY date_start", sessions, err))
        return false;

    while(sessions.step(err))
    {
        Session * const s = new Session(sessions["id"].get<int>(), err);
        if(err->code())
            return false;

        sessions_.push_back(s);
        vecSessionThreads_.push_back(thread(&Session::run, s));
    }

    return true;
}


// ambientTemp() - return a temperature reading from the ambient-temperature sensor, if present.  If no ambient-temp
// sensor is present, the function returns a Temperature object representing absolute zero.
//
Temperature SessionManager::ambientTemp() noexcept
{
    Temperature t = tempSensorAmbient_->sense();        // Always sense, to update the moving average

    return tempSensorAmbient_->inRange() ? t : Temperature();
}


// sessions() - return a vector of ptrs to the currently-active sessions
//
const vecSessionPtr_t& SessionManager::sessions() noexcept
{
    return sessions_;
}


// run() - main loop.
//
bool SessionManager::run() noexcept
{
    running_ = true;
    setName("smgr");

    display_.init();

    while(!stop_)
    {
        ambientTemp();      // Force an update of the ambient temperature moving average

        display_.update();
        ::usleep(10 * 1000);
    }

    logInfo("SessionManager stopping");

    for(auto session : sessions_)
        session->stop();

    display_.notifyShutdown();

    // Wait for session threads to terminate
    for(auto& sessionThread : vecSessionThreads_)
        sessionThread.join();

    display_.stop();
    running_ = false;

    return true;
}

