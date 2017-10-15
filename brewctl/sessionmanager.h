#ifndef SESSIONMANAGER_H_INC
#define SESSIONMANAGER_H_INC
/*
    sessionmanager.h: manages fermentation / conditioning sessions

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "sqlite.h"
#include "error.h"


class SessionManager
{
public:
                    SessionManager(SQLite& db);
    virtual         ~SessionManager();

    bool            init(Error * const err);

private:
    SQLite&         db_;
};

#endif // SESSIONMANAGER_H_INC

