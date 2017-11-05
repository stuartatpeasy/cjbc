#ifndef SQLITE_H_INC
#define SQLITE_H_INC
/*
    sqlite.h: minimal wrapper around common SQLite functions and management for a SQLite database object.

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
                    SQLite() noexcept;
    virtual         ~SQLite() noexcept;

    bool            open(const std::string& filename, const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                         Error * const err = nullptr) noexcept;
    bool            close(Error * const err = nullptr) noexcept;
    bool            isOpen() const noexcept { return db_ != nullptr; };
    bool            prepare(const std::string& sql, SQLiteStmt& stmt, Error * const err = nullptr) noexcept;
    bool            prepareAndStep(const std::string& sql, SQLiteStmt& stmt, Error * const err = nullptr) noexcept;

private:
    void            formatError(Error * const err, const int code) noexcept;
    sqlite3 *       db_;
    std::string     path_;
};

#endif // SQLITE_H_INC

