/*
    sessionmanager.cc: manages fermentation / conditioning sessions

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "sessionmanager.h"
#include "log.h"
#include "shiftreg.h"
#include "temperature.h"
#include "tempsensor.h"
#include <cstdlib>          // ::rand(), NULL
#include <ctime>            // ::strftime(), ::localtime(), ::time()

extern "C"
{
#include <unistd.h>         // ::usleep()
}


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
        Session * const s = new Session(sessions["id"], err);
        if(err->code())
            return false;

        sessions_.push_back(s);
    }

    return true;
}


// run() - main loop.
//
void SessionManager::run() noexcept
{
    Registry& r = Registry::instance();
    LCD& lcd = r.lcd();

    lcd.backlight(true);

    for(int i = 2; i < 4; ++i)
    {
        lcd.printAt(0, i, "F%d", i + 1);
    }

    lcd.printAt(14, 2, "10d18h");
    lcd.printAt(18, 0, "\xdf""C");

    for(int i = 0;; ++i)
    {
        tempSensorAmbient_->sense();

        for(auto session : sessions_)
            session->main();

        Temperature t = sessions_[0]->currentTemp();

        if(!(i % 100))
        {
            char buffer[16];
            time_t tm;

            ::time(&tm);
            ::strftime(buffer, sizeof(buffer), "%H:%M", ::localtime(&tm));

            lcd.printAt(0, 0, buffer);
            // FIXME - detect in-range for ambient sensor
            lcd.printAt(16, 0, "%2d", (int) tempSensorAmbient_->sense().C());

            for(auto session : sessions_)
            {
                lcd.printAt(0, 2, "G%-3d F ", session->gyleId());
                lcd.printAt(0, 3, "%.20s", session->gyleName().c_str());

                lcd.putAt(7, 2, LCD_CH_ARROW_2DOWN);
                if(session->vesselTempSensorInRange())
                    lcd.printAt(8, 2, "%4.1lf\xdf", t.C() + 0.05);
                else
                    lcd.printAt(8, 2, "--.-\xdf");
            }
        }

        ::usleep(10 * 1000);
    }
}

