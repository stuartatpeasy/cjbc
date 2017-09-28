/*
    sqlitestmt.cc: wraps a sqlite3_stmt object

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "sqlitestmt.h"
#include <cstdlib>

using std::string;


SQLiteStmt::SQLiteStmt()
    : stmt_(NULL)
{
}


SQLiteStmt::~SQLiteStmt()
{
    if(stmt_ != NULL)
        ::sqlite3_finalize(stmt_);
}


int SQLiteStmt::bind(const int index, const double arg)
{
    return ::sqlite3_bind_double(stmt_, index, arg);
}


int SQLiteStmt::bind(const int index, const int arg)
{
    return ::sqlite3_bind_int(stmt_, index, arg);
}


int SQLiteStmt::bind(const int index, const long long arg)
{
    return ::sqlite3_bind_int64(stmt_, index, arg);
}


int SQLiteStmt::bind(const int index, const string& arg)
{
    return ::sqlite3_bind_text(stmt_, index, arg.c_str(), arg.length(), SQLITE_TRANSIENT);
}


int SQLiteStmt::bind(const int index, const char * const arg)
{
    return ::sqlite3_bind_text(stmt_, index, arg, -1, SQLITE_TRANSIENT);
}


int SQLiteStmt::bind(const int index, const int len, const void * const arg)
{
    return ::sqlite3_bind_blob(stmt_, index, arg, len, SQLITE_TRANSIENT);
}


int SQLiteStmt::bindNull(const int index)
{
    return ::sqlite3_bind_null(stmt_, index);
}


int SQLiteStmt::clearBindings()
{
    return ::sqlite3_clear_bindings(stmt_);
}


int SQLiteStmt::step()
{
    return ::sqlite3_step(stmt_);
}


int SQLiteStmt::reset()
{
    return ::sqlite3_reset(stmt_);
}

