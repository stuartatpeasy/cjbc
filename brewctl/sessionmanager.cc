/*
    sessionmanager.cc: manages fermentation / conditioning sessions

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "sessionmanager.h"
#include "log.h"
#include "shiftreg.h"
#include "temperature.h"
#include <cstdlib>          // ::rand(), NULL

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

    if(!db.prepare("SELECT id "
                   "FROM session "
                   "WHERE date_start<=CURRENT_TIMESTAMP AND date_finish IS NULL "
                   "ORDER BY date_start", sessions, err))
        return false;

    while(sessions.step(err))
    {
        sessions_.push_back(new Session(sessions["id"], err));
        if(err->code())
            return false;
    }

    return !err->code();
}


// run() - main loop.
//
void SessionManager::run() noexcept
{
    Registry& r = Registry::instance();
    LCD& lcd = r.lcd();

    // Iterate every 10ms or so

    while(1)
    {
        for(auto it : sessions_)
        {
            Session& session = *it;

            session.main();
        }

        ::usleep(10 * 1000);
    }

    for(int i = 0; i < 2; ++i)
    {
        lcd.printAt(0, i, "F%d", i + 1);
        lcd.printAt(0, i + 2, "C%d", i + 1);
    }

    lcd.printAt(10, 0, "10d18h");
    lcd.printAt(4, 1, "--.-");
    lcd.printAt(4, 2, "--.-");
    lcd.printAt(4, 3, "--.-");

    for(int i = 0;; ++i)
    {
        Temperature t = sessions_[0]->currentTemp();

        if(!(i % 100))
        {
            if(t.C() > -5.0)
                lcd.printAt(4, 0, "%4.1lf\xdf", t.C() + 0.05);
            else
                lcd.printAt(4, 0, "--.- ");

            lcd.putAt(3, 0, LCD_CH_ARROW_2DOWN);
        }

        ::usleep(500 + (::rand() & 0x3ff));
    }
}

