/*
    sqlite.h: minimal wrapper around common SQLite functions and management for a SQLite database
    object.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "sqlite.h"
#include <cstdlib>
#include <memory>


SQLite::SQLite()
    : db_(nullptr)
{
}


SQLite::~SQLite()
{
}


int SQLite::open(const char * const filename, const int flags)
{
    int ret;

    ret = close();
    if(ret != SQLITE_OK)
        return ret;

    ret = ::sqlite3_open_v2(filename, &db_, flags, NULL);
    if(ret == SQLITE_OK)
        path_ = filename;

    return ret;
}


int SQLite::close()
{
    if(db_ != nullptr)
    {
        const int ret = sqlite3_close_v2(db_);
        if(ret == SQLITE_OK)
        {
            db_ = nullptr;
            path_ = "";
        }

        return ret;
    }

    return SQLITE_OK;
}


int SQLite::prepare(const char * const sql, SQLiteStmt& stmt)
{
    if(!isOpen())
        return SQLITE_ABORT;

    return ::sqlite3_prepare_v2(db_, sql, -1, stmt, NULL);
}

