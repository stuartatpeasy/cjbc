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
#include <cstdlib>          // ::rand()

extern "C"
{
#include <unistd.h>         // ::usleep()
}


SessionManager::SessionManager(Config& config, Error * const err)
    : config_(config), lcd_(gpio_)
{
    if(!db_.open(config_("database"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, err))
    {
        formatError(err, DB_OPEN_FAILED, config_("database").c_str(), err->message().c_str(),
                    err->code());
        return;
    }

    spi_ = new SPIPort(gpio_, config_, err);
    if(spi_ == nullptr)
    {
        formatError(err, MALLOC_FAILED);
        return;
    }

    if(err->code())
        return;

    adc_ = new ADC(&gpio_, spi_, config_, err);
    if(adc_ == nullptr)
    {
        formatError(err, MALLOC_FAILED);
        return;
    }

    if(err->code())
        return;
}


SessionManager::~SessionManager()
{
}


#include <iostream>     // FIXME remove
using namespace std;    // FIXME remove
bool SessionManager::init(Error * const err)
{
    SQLiteStmt sessions;

    if(db_.prepare("SELECT "
                       "id, gyle, profile_id, "
                       "CAST((JULIANDAY(date_start) - 2440587.5) * 86400.0 AS INT) AS start_ts "
                   "FROM session "
                   "WHERE date_start<=CURRENT_TIMESTAMP AND date_finish IS NULL "
                   "ORDER BY date_start", sessions, err))
    {
        while(sessions.step(err))
        {
            SQLiteStmt stages;

            cout << "Processing session " << sessions.column(1)->asCString() << endl;

            auto profileId = sessions.column(2);
            if(profileId == nullptr)
            {
               formatError(err, DB_TOO_FEW_COLUMNS);
               return false;
            }

            if(db_.prepare("SELECT * FROM profilestage "
                           "WHERE profile_id=:profile_id "
                           "ORDER BY stage_id",
                           stages, err))
            {
                if(!stages.bind(":profile_id", (int) *profileId, err))
                    return false;

                while(stages.step(err))
                {
                    logDebug("stepping stage");
                }
            }
        }

        if(err->code() && (err->code() != SQLITE_DONE))
            return false;
    }

    return true;
}


void SessionManager::run()
{
    Thermistor thermistor(3980, 4700, Temperature(25.0, TEMP_UNIT_CELSIUS));

    TempSensor sensor1(thermistor, *adc_, 0, 0.000147),
               sensor2(thermistor, *adc_, 1, 0.000147);

    ShiftReg sr(gpio_, *spi_);

    Temperature T1, T2;

    for(int i = 0; i < 2; ++i)
    {
        lcd_.printAt(0, i, "F%d", i + 1);
        lcd_.printAt(0, i + 2, "C%d", i + 1);
    }

    lcd_.printAt(10, 0, "10d18h");
    lcd_.printAt(4, 1, "--.-");
    lcd_.printAt(4, 2, "--.-");
    lcd_.printAt(4, 3, "--.-");

    sr.set(0);

    for(int i = 0;; ++i)
    {
        sensor1.sense(T1);
        sensor2.sense(T2);

        if(!(i % 100))
        {
            if(T1.C() > -5.0)
                lcd_.printAt(4, 0, "%4.1lf\xdf", T1.C() + 0.05);
            else
                lcd_.printAt(4, 0, "--.- ");

            if(T2.C() > -5.0)
                lcd_.printAt(4, 1, "%4.1lf\xdf", T2.C() + 0.05);
            else
                lcd_.printAt(4, 1, "--.- ");

            lcd_.putAt(3, 0, LCD_CH_ARROW_2DOWN);

            // XXX output switch 1 = bit 8 .... 8 = bit 15
            sr.toggle(10);
        }

        ::usleep(500 + (::rand() & 0x3ff));
    }
}

