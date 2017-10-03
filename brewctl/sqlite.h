#ifndef SQLITE_H_INC
#define SQLITE_H_INC
/*
    sqlite.h: minimal wrapper around common SQLite functions and management for a SQLite database
    object.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "error.h"
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
    virtual         ~SQLite();

    bool            open(const char * const filename,
                         const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                         Error * const err = nullptr);
    bool            close(Error * const err = nullptr);
    bool            isOpen() const { return db_ != nullptr; };
    bool            prepare(const char * const sql, SQLiteStmt& stmt, Error * const err = nullptr);
    bool            prepareAndStep(const char * const sql, SQLiteStmt& stmt,
                                   Error * const err = nullptr);

private:
    void            formatError(Error * const err, const int code);
    sqlite3 *       db_;
    std::string     path_;
};

#endif // SQLITE_H_INC

