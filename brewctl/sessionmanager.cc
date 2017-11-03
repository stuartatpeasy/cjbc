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

extern "C"
{
#include <unistd.h>         // ::usleep()
}


bool SessionManager::init(Error * const err)
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
        sessions_.push_back(Session(sessions["id"], err));
        if(err->code())
            return false;
    }

    return !err->code();
}


void SessionManager::run()
{
    Registry& r = Registry::instance();
    Thermistor thermistor(3980, 4700, Temperature(25.0, TEMP_UNIT_CELSIUS));

    TempSensor sensor1(thermistor, 0, 0.000147),
               sensor2(thermistor, 1, 0.000147);

    ShiftReg& sr = r.sr();
    Temperature T1, T2;
    LCD& lcd = r.lcd();

    for(int i = 0; i < 2; ++i)
    {
        lcd.printAt(0, i, "F%d", i + 1);
        lcd.printAt(0, i + 2, "C%d", i + 1);
    }

    lcd.printAt(10, 0, "10d18h");
    lcd.printAt(4, 1, "--.-");
    lcd.printAt(4, 2, "--.-");
    lcd.printAt(4, 3, "--.-");

    sr.set(0);

    for(int i = 0;; ++i)
    {
        sensor1.sense(T1);
        sensor2.sense(T2);

        if(!(i % 100))
        {
            if(T1.C() > -5.0)
                lcd.printAt(4, 0, "%4.1lf\xdf", T1.C() + 0.05);
            else
                lcd.printAt(4, 0, "--.- ");

            if(T2.C() > -5.0)
                lcd.printAt(4, 1, "%4.1lf\xdf", T2.C() + 0.05);
            else
                lcd.printAt(4, 1, "--.- ");

            lcd.putAt(3, 0, LCD_CH_ARROW_2DOWN);

            // XXX output switch 1 = bit 8 .... 8 = bit 15
            sr.toggle(10);
        }

        ::usleep(500 + (::rand() & 0x3ff));
    }
}

