/*
    sqlitecolumn.cc: abstraction for a column value in a result set returned from a SQLite query

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "sqlitecolumn.h"
#include "log.h"
#include "sqlitestmt.h"
#include <cstring>      // memcpy()


SQLiteColumn::SQLiteColumn(SQLiteStmt &stmt, const int index) noexcept
    : index_(index), data_(nullptr), len_(0)
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


// dtor - free data buffer
//
SQLiteColumn::~SQLiteColumn() noexcept
{
    if(data_ != nullptr)
        delete[] data_;
}


// copy() - copy <rhs> into this object
//
SQLiteColumn& SQLiteColumn::copy(const SQLiteColumn& rhs) noexcept
{
    index_ = rhs.index_;
    isNull_ = rhs.isNull_;
    len_ = rhs.len_;

    if(!isNull_)
    {
        if(data_ != nullptr)
            delete[] data_;

        data_ = new char[len_ + 1];
        if(data_ != nullptr)
        {
            ::memcpy(rhs.data_, data_, len_);
            data_[len_] = '\0';
        }
        else
        {
            // XXX This is pretty bad.  If buffer allocation fails, we have little recourse (in the absence of
            // exceptions) other than to log an error and make the copy of the source object an SQL NULL.
            logError("SQLite column init: failed to allocate %d bytes; creating an SQL NULL instead!", len_);

            len_ = 0;
            isNull_ = true;
        }
    }

    return *this;
}


// move() - move <rhs> into this object
//
SQLiteColumn& SQLiteColumn::move(SQLiteColumn& rhs) noexcept
{
    if(data_ != nullptr)
        delete[] data_;

    index_ = rhs.index_;
    data_ = rhs.data_;
    len_ = rhs.len_;
    isNull_ = rhs.isNull_;

    rhs.len_ = 0;
    rhs.data_ = nullptr;

    return *this;
}

