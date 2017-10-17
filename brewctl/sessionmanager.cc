/*
    sessionmanager.cc: manages fermentation / conditioning sessions

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "sessionmanager.h"
#include "log.h"


SessionManager::SessionManager(Config& config, Error * const err)
    : config_(config)
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


bool SessionManager::init(Error * const err)
{
    SQLiteStmt sessions;

    if(db_.prepare("SELECT id, gyle, type FROM session WHERE date_start<=CURRENT_TIMESTAMP",
                   sessions, err))
    {
        while(sessions.step(err))
        {
            SQLiteStmt stages;

            auto sessionId = sessions.column(0);
            if(sessionId == nullptr)
            {
               formatError(err, DB_TOO_FEW_COLUMNS);
               return false;
            }

            if(db_.prepare("SELECT * FROM sessionstage WHERE session_id=:id ORDER BY stage_id",
                           stages, err))
            {
                if(!stages.bind(":id", (int) *sessionId, err))
                    return false;

                while(stages.step(err))
                {
                    logDebug("stepping stage");
                }
            }
        }

        if(err->code() != SQLITE_DONE)
            return false;
    }

    return true;
}

