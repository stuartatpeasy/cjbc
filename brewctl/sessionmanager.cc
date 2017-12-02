/*
    sessionmanager.cc: manages fermentation / conditioning sessions

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "include/sessionmanager.h"
#include "include/framework/log.h"
#include "include/peripherals/shiftreg.h"
#include "include/peripherals/tempsensor.h"
#include "include/temperature.h"
#include <cstdlib>          // ::rand(), NULL
#include <ctime>            // ::strftime(), ::localtime(), ::time()
#include <thread>

extern "C"
{
#include <unistd.h>         // ::usleep()
}

using std::thread;


// Time-interval constants
static const time_t 
    HOURS_PER_DAY                   = 24,
    MINS_PER_HOUR                   = 60,
    SECS_PER_MINUTE                 = 60,
    SECS_PER_HOUR                   = MINS_PER_HOUR * SECS_PER_MINUTE,
    SECS_PER_DAY                    = HOURS_PER_DAY * SECS_PER_HOUR,
    DEFAULT_DISPLAY_UPDATE_INTERVAL = 1;


// ctor - trivial initialisation of members
//
SessionManager::SessionManager() noexcept
    : Thread(),
      lastDisplayUpdate_(0),
      displayUpdateInterval_(DEFAULT_DISPLAY_UPDATE_INTERVAL)
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
        thread(&Session::run, s).detach();
    }

    return true;
}


// getTempControlIndicator() - return a character representing the temperature-control activity specified by <state>,
// e.g. an up-arrow character to indicate that the session is currently heating its vessel.
//
char SessionManager::getTempControlIndicator(const SessionTempControlState_t state) const noexcept
{
    switch(state)
    {
        case FAST_COOL:     return LCD_CH_ARROW_2DOWN;
        case COOL:          return LCD_CH_ARROW_DOWN;
        case HOLD:          return LCD_CH_ARROW_RIGHT;
        case HEAT:          return LCD_CH_ARROW_UP;
        case FAST_HEAT:     return LCD_CH_ARROW_2UP;
        default:            return '?';
    }
}


// getSessionTypeIndicator() - return a character representing the type (fermentation or conditioning) specified by
// <type>.
//
char SessionManager::getSessionTypeIndicator(const SessionType_t type) const noexcept
{
    switch(type)
    {
        case FERMENT:       return 'F';
        case CONDITION:     return 'C';
        default:            return '?';
    }
}


// run() - main loop.
//
void SessionManager::run() noexcept
{
    running_ = true;
    setName("smgr");

    LCD& lcd = Registry::instance().lcd();

    lcd.backlight(true);

    lcd.printAt(18, 0, "\xdf""C");

    while(1)
    {
        const time_t now = ::time(NULL);
        tempSensorAmbient_->sense();

        if((now - lastDisplayUpdate_) > displayUpdateInterval_)
        {
            char buffer[16];
            time_t tm;

            lastDisplayUpdate_ = now;

            ::time(&tm);
            ::strftime(buffer, sizeof(buffer), "%H:%M", ::localtime(&tm));
            lcd.printAt(0, 0, buffer);

            if(tempSensorAmbient_->inRange())
                lcd.printAt(16, 0, "%2d", (int) (tempSensorAmbient_->sense().C() + 0.5));
            else
                lcd.printAt(16, 0, "--");

            for(auto session : sessions_)
            {
                lcd.printAt(0, 2, "G%-3d", session->gyleId());
                lcd.printAt(0, 3, "%.20s", session->gyleName().c_str());
                
                if(session->isActive())
                {
                    lcd.putAt(5, 2, getSessionTypeIndicator(session->type()));

                    lcd.putAt(7, 2, getTempControlIndicator(session->tempControlState()));
                    if(session->vesselTempSensorInRange())
                        lcd.printAt(8, 2, "%4.1lf\xdf", session->currentTemp().C());
                    else
                        lcd.printAt(8, 2, "--.-\xdf");

                    const time_t secsRemaining  = session->remainingTime(),
                                 days           = secsRemaining / SECS_PER_DAY,
                                 hours          = secsRemaining / SECS_PER_HOUR,
                                 minutes        = secsRemaining / SECS_PER_MINUTE;

                    time_t field1, field2;
                    const char *fmt;

                    if(days)
                    {
                        fmt = "%2dd%02dh";
                        field1 = days;
                        field2 = hours % HOURS_PER_DAY;
                    }
                    else if(hours)
                    {
                        fmt = "%2dh%02dm";
                        field1 = hours;
                        field2 = minutes % MINS_PER_HOUR;
                    }
                    else
                    {
                        fmt = "%2dm%02ds";
                        field1 = minutes;
                        field2 = secsRemaining % SECS_PER_MINUTE;
                    }

                    lcd.printAt(14, 2, fmt, field1, field2);
                }
                else
                {
                    lcd.printAt(5, 0, session->isComplete() ? "Complete" : "Starts in");
                }
            }
        }

        ::usleep(10 * 1000);
    }
}

