/*
    sqlitestmt.cc: wraps a sqlite3_stmt object

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/sqlitestmt.h"
#include "include/log.h"
#include <cstdlib>
#include <initializer_list>

using std::lock_guard;
using std::mutex;
using std::string;
using std::unique_ptr;
using std::vector;


mutex SQLiteStmt::lock_;


SQLiteStmt::SQLiteStmt() noexcept
    : stmt_(NULL),
      firstStepDone_(false)
{
}


// dtor - finalise the current statement, if any.
//
SQLiteStmt::~SQLiteStmt() noexcept
{
    finalise();
}

//
// Indexed bind methods
//

// bind() - bind the double value <arg> to variable <index> in the current statement.
//
bool SQLiteStmt::bind(const int index, const double arg, Error * const err) noexcept
{
    return checkError(::sqlite3_bind_double(stmt_, index, arg), err);
}


// bind() - bind the int value <arg> to variable <index> in the current statement.
//
bool SQLiteStmt::bind(const int index, const int arg, Error * const err) noexcept
{
    return checkError(::sqlite3_bind_int(stmt_, index, arg), err);
}


// bind() - bind the long-long value <arg> to variable <index> in the current statement.
//
bool SQLiteStmt::bind(const int index, const long long arg, Error * const err) noexcept
{
    return checkError(::sqlite3_bind_int64(stmt_, index, arg), err);
}


// bind() - bind the string value <arg> to variable <index> in the current statement.
//
bool SQLiteStmt::bind(const int index, const string& arg, Error * const err) noexcept
{
    return checkError(::sqlite3_bind_text(stmt_, index, arg.c_str(), arg.length(), SQLITE_TRANSIENT), err);
}


// bind() - bind a BLOB value at <arg>, with length <len>, to variable <index> in the current
// statement.
//
bool SQLiteStmt::bind(const int index, const int len, const void * const arg, Error * const err) noexcept
{
    return checkError(::sqlite3_bind_blob(stmt_, index, arg, len, SQLITE_TRANSIENT), err);
}


// bindNull() - bind an SQL NULL value to variable <index> in the current statement.
//
bool SQLiteStmt::bindNull(const int index, Error * const err) noexcept
{
    return checkError(::sqlite3_bind_null(stmt_, index), err);
}

//
// Named-parameter bind methods
//

// bind() - bind the double value <arg> to the named parameter <param> in the current statement.
//
bool SQLiteStmt::bind(const string& param, const double arg, Error * const err) noexcept
{
    // ::sqlite_bind_*() will return SQLITE_RANGE if <param> specifies an invalid param.
    return bind(::sqlite3_bind_parameter_index(stmt_, param.c_str()), arg, err);
}

// bind() - bind the double value <arg> to the named parameter <param> in the current statement.
//
bool SQLiteStmt::bind(const string& param, const int arg, Error * const err) noexcept
{
    // ::sqlite_bind_*() will return SQLITE_RANGE if <param> specifies an invalid param.
    return bind(::sqlite3_bind_parameter_index(stmt_, param.c_str()), arg, err);
}


// bind() - bind the double value <arg> to the named parameter <param> in the current statement.
//
bool SQLiteStmt::bind(const string& param, const long long arg, Error * const err) noexcept
{
    // ::sqlite_bind_*() will return SQLITE_RANGE if <param> specifies an invalid param.
    return bind(::sqlite3_bind_parameter_index(stmt_, param.c_str()), arg, err);
}


// bind() - bind the double value <arg> to the named parameter <param> in the current statement.
//
bool SQLiteStmt::bind(const string& param, const std::string& arg, Error * const err) noexcept
{
    // ::sqlite_bind_*() will return SQLITE_RANGE if <param> specifies an invalid param.
    return bind(::sqlite3_bind_parameter_index(stmt_, param.c_str()), arg, err);
}


// bind() - bind a BLOB value at <arg>, with length <len>,  to the named parameter <param> in the current statement.
//
bool SQLiteStmt::bind(const string& param, const int len, const void * const arg, Error * const err) noexcept
{
    // ::sqlite_bind_*() will return SQLITE_RANGE if <param> specifies an invalid param.
    return bind(::sqlite3_bind_parameter_index(stmt_, param.c_str()), len, arg, err);
}


// bindNull() - bind an SQL NULL value to the named parameter <param> in the current statement.
//
bool SQLiteStmt::bindNull(const string& param, Error * const err) noexcept
{
    // ::sqlite_bind_*() will return SQLITE_RANGE if <param> specifies an invalid param.
    return bindNull(::sqlite3_bind_parameter_index(stmt_, param.c_str()), err);
}


// clearBindings() - remove all variable-to-column bindings from the current statement.
//
bool SQLiteStmt::clearBindings(Error * const err) noexcept
{
    return checkError(::sqlite3_clear_bindings(stmt_), err);
}


// numCols() - return the number of columns associated with the current statement, or 0 if there is no current
// statement.
//
int SQLiteStmt::numCols() noexcept
{
    return ::sqlite3_data_count(stmt_);             // safe even if stmt_ == NULL
}


// step() - advance the current prepared statement result-set to the next record, if any.  Return true if another record
// was found.  Return false and do not set <err> if there are no more records.  Return false and set <err> if anything
// else went wrong.
//
bool SQLiteStmt::step(Error * const err) noexcept
{
    const int ret = ::sqlite3_step(stmt_);
    if(ret == SQLITE_DONE)
        return false;       // No more records; do not set err.

    if(checkError(ret, err, SQLITE_ROW))
    {
        if(!firstStepDone_)
        {
            // This is the first call to step() on the current statement, or the first call since a call to finalise()
            // or reset().  Build a list of column names in the result set.
            getColumnNames();
            firstStepDone_ = true;
        }

        return true;
    }

    return false;
}


// reset() - reset the current statement to its initial state, preserving any existing variable-to-column bindings.
//
bool SQLiteStmt::reset(Error * const err) noexcept
{
    lock_guard<mutex> lock(lock_);
    if(checkError(::sqlite3_reset(stmt_), err))
    {
        columnNames_.clear();
        firstStepDone_ = false;
        return true;
    }

    return false;
}


// column() - obtain column <index> from the current record in the result set for the current statement.  Return a
// "fake" column, containing an SQL NULL value, if <index> is out-of-bounds.
//
SQLiteColumn SQLiteStmt::column(const int index) noexcept
{
    if((index < 0) || (index >= numCols()))
    {
        logError("SQLiteStmt: stmt {%x}: requested out-of-range column %d; returning NULL column", id(), index);
        return SQLiteColumn(*this, -1);
    }

    return SQLiteColumn(*this, index);
}


// column() - obtain the column called <name> from the current record in the result set for the current statement.
// Return a "fake" column, containing an SQL NULL value, if <index> is out-of-bounds.
//
SQLiteColumn SQLiteStmt::column(const string& name) noexcept
{
    const auto n = columnNames_.find(name);
    if(n == columnNames_.end())
    {
        logError("SQLiteStmt: stmt {%x}: requested unknown column '%s'; returning NULL column", id(), name.c_str());
        return SQLiteColumn(*this, -1);
    }

    return SQLiteColumn(*this, n->second);
}


// finalise() - "finalise", i.e. destruct, a stmt.
//
void SQLiteStmt::finalise() noexcept
{
    if(stmt_ != NULL)
    {
        logDebug("SQLiteStmt: stmt {%x}: finalizing", id());
        ::sqlite3_finalize(stmt_);
        stmt_ = NULL;
        columnNames_.clear();
        firstStepDone_ = false;
    }
}


// checkError() - given the result of a call to an sqlite3_*() fn in <ret>, and a value indicating that the call
// succeeded in <successCode>, return true if <ret> == <successCode> (i.e. the call succeeded); otherwise, if <err> is
// non-null then populate it with an appropriate error code and message, and return false.
//
bool SQLiteStmt::checkError(const int ret, Error * const err, const int successCode) noexcept
{
    vector<int> codes;
    codes.push_back(successCode);
    return checkError(ret, err, codes);
}


// checkError() - given the result of a call to an sqlite3_*() fn in <ret>, and set of values indicating that the call
// succeeded in <successCodes>, return try if <ret> is present in <successCodes> (i.e. the call succeeded); otherwise,
// if <err> is non-null the populate it with an appropriate error code and message, and return false.
//
bool SQLiteStmt::checkError(const int ret, Error * const err, vector<int> successCodes) noexcept
{
    for(auto code : successCodes)
        if(ret == code)
            return true;

    formatError(err, ret);
    return false;
}


// formatError() - populate Error object err (if non-null) with the supplied error code and an appropriate
// human-readable error message.
//
void SQLiteStmt::formatError(Error * const err, const int code) noexcept
{
    // Log any errors.  Exclude return values which do not indicate an error.
    if(code != SQLITE_DONE)
        logWarning("SQLite stmt {%x}: error %d: %s", id(), code, ::sqlite3_errstr(code));

    ::formatError(err, DB_SQLITESTMT_ERROR, ::sqlite3_errstr(code), code);
}


// getColumnNames() - attempt to build into <columnNames_> a list of names of columns in the result-set.  Try to
// retrieve assigned names (names to the right of an "... AS" expression) first; if this fails (e.g. if a column has no
// "AS" clause), try to retrieve "original names".  If a column name cannot be retrieved by either of these methods,
// no entry for the column will be made in <columnNames_>.
//
void SQLiteStmt::getColumnNames() noexcept
{
    columnNames_.clear();
    for(int i = 0; i < numCols(); ++i)
    {
        // Try sqlite3_column_name() first.  This will return NULL if the column has not been explicitly named using an
        // "AS ..." clause; otherwise it will return the name to the right of the "AS" keyword.
        const char * name = ::sqlite3_column_name(stmt_, i);
        if(name == NULL)
        {
            // sqlite3_column_name() returned NULL; try sqlite3_column_origin_name() instead.  This will fail if the
            // column is an expression or a sub-SELECT, or if a memory-allocation fails.
            // assigned to an expression or sub-query column.
            name = ::sqlite3_column_origin_name(stmt_, i);
        }

        // If name is still NULL at this point, we really don't know the name of this column.
        if(name != NULL)
            columnNames_[name] = i;
    }
}

