/*
    sqlitecolumn.cc: abstraction for a column value in a result set returned from a SQLite query

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "sqlitecolumn.h"
#include "sqlitestmt.h"
#include <cstring>      // memcpy()


SQLiteColumn::SQLiteColumn(SQLiteStmt &stmt, const int index)
    : index_(index), data_(nullptr), len_(0)
{
    sqlite3_stmt * const stmtRaw = (sqlite3_stmt *) stmt;

    if(::sqlite3_column_type(stmtRaw, index) == SQLITE_NULL)
        isNull_ = true;
    else
    {
        isNull_ = false;

        const int len = ::sqlite3_column_bytes(stmtRaw, index);

        data_ = new char[len + 1];
        if(data_ != nullptr)
        {
            const void *p = ::sqlite3_column_blob(stmtRaw, index);

            if(p != NULL)
            {
                ::memcpy(data_, p, len);
                data_[len] = '\0';
                len_ = len;
            }
        }
    }
}


SQLiteColumn::~SQLiteColumn()
{
    if(data_ != nullptr)
        delete[] data_;
}

