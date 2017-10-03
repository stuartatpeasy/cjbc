#ifndef DATABASE_H_INC
#define DATABASE_H_INC
/*
    database.h: very light wrapper around SQLite, to provide persistent database storage.
    Implemented as a singleton.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "error.h"
#include "sqlite.h"
#include <string>
#include <memory>

extern "C"
{
#include <sqlite3.h>
}


class Database
{
                    Database();
    virtual         ~Database();

    bool            open(const char * const path, Error& err);

protected:
    SQLite                              sqlite_;
};


#endif // DATABASE_H_INC
