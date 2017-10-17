/*
    sqlitestmt.cc: wraps a sqlite3_stmt object

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "sqlitestmt.h"
#include "log.h"
#include <cstdlib>
#include <initializer_list>

using std::string;
using std::unique_ptr;
using std::vector;


SQLiteStmt::SQLiteStmt()
    : stmt_(NULL)
{
}


// dtor - finalise the current statement, if any.
//
SQLiteStmt::~SQLiteStmt()
{
    if(stmt_ != NULL)
    {
        logDebug("SQLiteStmt: stmt %x: finalizing", id());
        ::sqlite3_finalize(stmt_);
    }
}

//
// Indexed bind methods
//

// bind() - bind the double value <arg> to variable <index> in the current statement.
//
bool SQLiteStmt::bind(const int index, const double arg, Error * const err)
{
    return checkError(::sqlite3_bind_double(stmt_, index, arg), err);
}


// bind() - bind the int value <arg> to variable <index> in the current statement.
//
bool SQLiteStmt::bind(const int index, const int arg, Error * const err)
{
    return checkError(::sqlite3_bind_int(stmt_, index, arg), err);
}


// bind() - bind the long-long value <arg> to variable <index> in the current statement.
//
bool SQLiteStmt::bind(const int index, const long long arg, Error * const err)
{
    return checkError(::sqlite3_bind_int64(stmt_, index, arg), err);
}


// bind() - bind the string value <arg> to variable <index> in the current statement.
//
bool SQLiteStmt::bind(const int index, const string& arg, Error * const err)
{
    return checkError(::sqlite3_bind_text(stmt_, index, arg.c_str(), arg.length(),
                                          SQLITE_TRANSIENT), err);
}


// bind() - bind a BLOB value at <arg>, with length <len>, to variable <index> in the current
// statement.
//
bool SQLiteStmt::bind(const int index, const int len, const void * const arg, Error * const err)
{
    return checkError(::sqlite3_bind_blob(stmt_, index, arg, len, SQLITE_TRANSIENT), err);
}


// bindNull() - bind an SQL NULL value to variable <index> in the current statement.
//
bool SQLiteStmt::bindNull(const int index, Error * const err)
{
    return checkError(::sqlite3_bind_null(stmt_, index), err);
}

//
// Named-parameter bind methods
//

// bind() - bind the double value <arg> to the named parameter <param> in the current statement.
//
bool SQLiteStmt::bind(const string& param, const double arg, Error * const err)
{
    // ::sqlite_bind_*() will return SQLITE_RANGE if <param> specifies an invalid param.
    return bind(::sqlite3_bind_parameter_index(stmt_, param.c_str()), arg, err);
}

// bind() - bind the double value <arg> to the named parameter <param> in the current statement.
//
bool SQLiteStmt::bind(const string& param, const int arg, Error * const err)
{
    // ::sqlite_bind_*() will return SQLITE_RANGE if <param> specifies an invalid param.
    return bind(::sqlite3_bind_parameter_index(stmt_, param.c_str()), arg, err);
}


// bind() - bind the double value <arg> to the named parameter <param> in the current statement.
//
bool SQLiteStmt::bind(const string& param, const long long arg, Error * const err)
{
    // ::sqlite_bind_*() will return SQLITE_RANGE if <param> specifies an invalid param.
    return bind(::sqlite3_bind_parameter_index(stmt_, param.c_str()), arg, err);
}


// bind() - bind the double value <arg> to the named parameter <param> in the current statement.
//
bool SQLiteStmt::bind(const string& param, const std::string& arg, Error * const err)
{
    // ::sqlite_bind_*() will return SQLITE_RANGE if <param> specifies an invalid param.
    return bind(::sqlite3_bind_parameter_index(stmt_, param.c_str()), arg, err);
}


// bind() - bind a BLOB value at <arg>, with length <len>,  to the named parameter <param> in the
// current statement.
//
bool SQLiteStmt::bind(const string& param, const int len, const void * const arg,
                      Error * const err)
{
    // ::sqlite_bind_*() will return SQLITE_RANGE if <param> specifies an invalid param.
    return bind(::sqlite3_bind_parameter_index(stmt_, param.c_str()), len, arg, err);
}


// bindNull() - bind an SQL NULL value to the named parameter <param> in the current statement.
//
bool SQLiteStmt::bindNull(const string& param, Error * const err)
{
    // ::sqlite_bind_*() will return SQLITE_RANGE if <param> specifies an invalid param.
    return bindNull(::sqlite3_bind_parameter_index(stmt_, param.c_str()), err);
}


// clearBindings() - remove all variable-to-column bindings from the current statement.
//
bool SQLiteStmt::clearBindings(Error * const err)
{
    return checkError(::sqlite3_clear_bindings(stmt_), err);
}


// numCols() - return the number of columns associated with the current statement, or 0 if there is
// no current statement.
//
int SQLiteStmt::numCols()
{
    return ::sqlite3_data_count(stmt_);             // safe even if stmt_ == NULL
}


// step() - advance the current prepared statement result-set to the next record, if any.  Return
// true if another record was found.  Return false and do not set <err> if there are no more
// records.  Return false and set <err> if anything else went wrong.
//
bool SQLiteStmt::step(Error * const err)
{
    const int ret = ::sqlite3_step(stmt_);
    if(ret == SQLITE_DONE)
        return false;       // No more records; do not set err.

    return checkError(ret, err, SQLITE_ROW);
}


// reset() - reset the current statement to its initial state, preserving any existing variable-to-
// column bindings.
//
bool SQLiteStmt::reset(Error * const err)
{
    return checkError(::sqlite3_reset(stmt_), err);
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


// checkError() - given the result of a call to an sqlite3_*() fn in <ret>, and a value indicating
// that the call succeeded in <successCode>, return true if <ret> == <successCode> (i.e. the call
// succeeded); otherwise, if <err> is non-null then populate it with an appropriate error code and
// message, and return false.
//
bool SQLiteStmt::checkError(const int ret, Error * const err, const int successCode)
{
    vector<int> codes;
    codes.push_back(successCode);
    return checkError(ret, err, codes);
}


// checkError() - given the result of a call to an sqlite3_*() fn in <ret>, and set of values
// indicating that the call succeeded in <successCodes>, return try if <ret> is present in
// <successCodes> (i.e. the call succeeded); otherwise, if <err> is non-null the populate it with an
// appropriate error code and message, and return false.
//
bool SQLiteStmt::checkError(const int ret, Error * const err, vector<int> successCodes)
{
    for(auto code : successCodes)
        if(ret == code)
            return true;

    formatError(err, ret);
    return false;
}


// formatError() - populate Error object err (if non-null) with the supplied error code and an
// appropriate human-readable error message.
//
void SQLiteStmt::formatError(Error * const err, const int code)
{
    // Log any errors.  Exclude return values which do not indicate an error.
    if(code != SQLITE_DONE)
        logWarning("SQLite stmt %x: error %d: %s", id(), code, ::sqlite3_errstr(code));

    ::formatError(err, DB_SQLITESTMT_ERROR, ::sqlite3_errstr(code), code);
}

