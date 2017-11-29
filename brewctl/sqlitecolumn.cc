/*
    sqlitecolumn.cc: abstraction for a column value in a result set returned from a SQLite query

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "sqlitecolumn.h"
#include "log.h"
#include "sqlitestmt.h"


// ctor - read column data from the statement object; determine the length of the data and whether it represents a SQL
// NULL value.  The SQLiteStmt object, <stmt>, is not needed once the ctor returns; it can be finalise()d, etc.
//
SQLiteColumn::SQLiteColumn(SQLiteStmt &stmt, const int index) noexcept
    : index_(index),
      len_(0)
{
    sqlite3_stmt * const stmtRaw = (sqlite3_stmt *) stmt;

    // A negative value can be passed in <index>; in this case, the requested column is out of range and a fake column
    // is instantiated instead.  The fake column always contains an SQL NULL value.
    if((index < 0) || (::sqlite3_column_type(stmtRaw, index) == SQLITE_NULL))
        isNull_ = true;
    else
    {
        isNull_ = false;

        const int len = ::sqlite3_column_bytes(stmtRaw, index);
        const void *p = ::sqlite3_column_blob(stmtRaw, index);

        if(p != NULL)
        {
            value_.set((const char *) p, len);
            len_ = len;
        }
    }
}


// copy ctor
//
SQLiteColumn::SQLiteColumn(const SQLiteColumn& rhs) noexcept
{
    copy(rhs);
}


// move ctor - take ownership of rhs
//
SQLiteColumn::SQLiteColumn(SQLiteColumn&& rhs) noexcept
{
    move(rhs);
}


// assignment operator
//
SQLiteColumn& SQLiteColumn::operator=(const SQLiteColumn& rhs) noexcept
{
    return copy(rhs);
}


SQLiteColumn& SQLiteColumn::operator=(SQLiteColumn&& rhs) noexcept
{
    return move(rhs);
}


// copy() - copy <rhs> into this object
//
SQLiteColumn& SQLiteColumn::copy(const SQLiteColumn& rhs) noexcept
{
    index_  = rhs.index_;
    isNull_ = rhs.isNull_;
    len_    = rhs.len_;
    value_  = rhs.value_;

    return *this;
}


// move() - move <rhs> into this object
//
SQLiteColumn& SQLiteColumn::move(SQLiteColumn& rhs) noexcept
{
    index_  = rhs.index_;
    len_    = rhs.len_;
    isNull_ = rhs.isNull_;
    value_  = rhs.value_;

    rhs.len_ = 0;
    rhs.value_.clear();

    return *this;
}

