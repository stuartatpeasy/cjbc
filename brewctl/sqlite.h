#ifndef SQLITE_H_INC
#define SQLITE_H_INC
/*
    sqlite.h: minimal wrapper around common SQLite functions and management for a SQLite database
    object.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "sqlitestmt.h"
#include <string>


extern "C"
{
#include <sqlite3.h>
}


class SQLite
{
public:
                            SQLite();
    virtual                 ~SQLite();

    int                     open(const char * const filename, const int flags = 0);
    int                     close();
    bool                    isOpen() const { return db_ != nullptr; };
    int                     prepare(const char * const sql, SQLiteStmt& stmt);

private:
    sqlite3 *               db_;
    std::string             path_;
};

#endif // SQLITE_H_INC

