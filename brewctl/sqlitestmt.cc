/*
    sqlitestmt.cc: wraps a sqlite3_stmt object

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "sqlitestmt.h"
#include <cstdlib>

using std::string;
using std::unique_ptr;


SQLiteStmt::SQLiteStmt()
    : stmt_(NULL)
{
}


// dtor - finalise the current statement, if any.
//
SQLiteStmt::~SQLiteStmt()
{
    if(stmt_ != NULL)
        ::sqlite3_finalize(stmt_);
}


// bind() - bind the double value <arg> to variable <index> in the current statement.
//
int SQLiteStmt::bind(const int index, const double arg)
{
    return ::sqlite3_bind_double(stmt_, index, arg);
}


// bind() - bind the int value <arg> to variable <index> in the current statement.
//
int SQLiteStmt::bind(const int index, const int arg)
{
    return ::sqlite3_bind_int(stmt_, index, arg);
}


// bind() - bind the long-long value <arg> to variable <index> in the current statement.
//
int SQLiteStmt::bind(const int index, const long long arg)
{
    return ::sqlite3_bind_int64(stmt_, index, arg);
}


// bind() - bind the string value <arg> to variable <index> in the current statement.
//
int SQLiteStmt::bind(const int index, const string& arg)
{
    return ::sqlite3_bind_text(stmt_, index, arg.c_str(), arg.length(), SQLITE_TRANSIENT);
}


// bind() - bind the zero-terminated string value <arg> to variable <index> in the current
// statement.
//
int SQLiteStmt::bind(const int index, const char * const arg)
{
    return ::sqlite3_bind_text(stmt_, index, arg, -1, SQLITE_TRANSIENT);
}


// bind() - bind a BLOB value at <arg>, with length <len>, to variable <index> in the current
// statement.
//
int SQLiteStmt::bind(const int index, const int len, const void * const arg)
{
    return ::sqlite3_bind_blob(stmt_, index, arg, len, SQLITE_TRANSIENT);
}


// bindNull() - bind an SQL NULL value to variable <index> in the current statement.
//
int SQLiteStmt::bindNull(const int index)
{
    return ::sqlite3_bind_null(stmt_, index);
}


// clearBindings() - remove all variable-to-column bindings from the current statement.
//
int SQLiteStmt::clearBindings()
{
    return ::sqlite3_clear_bindings(stmt_);
}


// numCols() - return the number of columns associated with the current statement, or 0 if there is
// no current statement.
//
int SQLiteStmt::numCols()
{
    return ::sqlite3_data_count(stmt_);             // safe even if stmt_ == NULL
}


// step() - advance the current prepared statement result-set to the next record, if any.
//
int SQLiteStmt::step()
{
    return ::sqlite3_step(stmt_);
}


// reset() - reset the current statement to its initial state, preserving any existing variable-to-
// column bindings.
//
int SQLiteStmt::reset()
{
    return ::sqlite3_reset(stmt_);
}


// column() - obtain column <index> from the current record in the result set for the current
// statement.  Return nullptr if <index> is out-of-bounds.
//
unique_ptr<SQLiteColumn> SQLiteStmt::column(const int index)
{
    if((index < 0) || (index >= numCols()))
        return nullptr;

    unique_ptr<SQLiteColumn> col(new SQLiteColumn(*this, index));

    return col;
}

