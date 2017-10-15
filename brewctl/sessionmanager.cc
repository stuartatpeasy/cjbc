/*
    sessionmanager.cc: manages fermentation / conditioning sessions

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "sessionmanager.h"
#include "log.h"


SessionManager::SessionManager(SQLite& db)
    : db_(db)
{
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
                return err->format(DB_TOO_FEW_COLUMNS);

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

