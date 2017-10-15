/*
    sqlite.h: minimal wrapper around common SQLite functions and management for a SQLite database
    object.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "sqlite.h"
#include <cstdlib>
#include <memory>

using std::string;


SQLite::SQLite()
    : db_(nullptr)
{
}


SQLite::~SQLite()
{
    close();
}


// open() - attempt to open a new database session using the database file <filename>.
//
bool SQLite::open(const string& filename, const int flags, Error * const err)
{
    if(!close(err))
        return false;

    const int ret = ::sqlite3_open_v2(filename.c_str(), &db_, flags, NULL);
    if(ret == SQLITE_OK)
    {
        path_ = filename;
        return true;
    }

    formatError(err, ret);

    return false;
}


// close() - attempt to close the currently-open database session.  Returns true on success, or if
// there is no currently-open session; false otherwise.
//
bool SQLite::close(Error * const err)
{
    if(db_ == nullptr)
        return true;        // Database not open - return success

    const int ret = ::sqlite3_close_v2(db_);
    if(ret == SQLITE_OK)
    {
        db_ = nullptr;
        path_ = "";
        return true;
    }

    formatError(err, ret);
    return false;
}


// prepare() - prepare the SQL statement <sql> for execution; on success, return a statement object
// through <stmt>.  Return the SQLite retval in all cases.
//
bool SQLite::prepare(const string& sql, SQLiteStmt& stmt, Error * const err)
{
    int ret;

    if(isOpen())
    {
        ret = ::sqlite3_prepare_v2(db_, sql.c_str(), -1, stmt, NULL);

        if(ret == SQLITE_OK)
            return true;
    }
    else
        ret = SQLITE_ABORT;

    formatError(err, ret);
    return false;
}


// prepareAndStep() - prepare the SQL query in <sql> and - if successful - execute step() on the
// prepared statement in order to execute the query and generate the first resulting record.  This
// method is intended for use with queries in which the result-set is not required.
//
bool SQLite::prepareAndStep(const std::string& sql, SQLiteStmt& stmt, Error * const err)
{
    if(!prepare(sql, stmt, err))
        return false;

    const int ret = stmt.step();
    if(ret == SQLITE_OK)
        return true;

    formatError(err, ret);
    return false;
}


// formatError() - populate Error object err (if non-null) with the supplied error code and an
// appropriate human-readable error message.
//
void SQLite::formatError(Error * const err, const int code)
{
    if(err != nullptr)
        err->format(code, "SQLite error %d: %s", code, ::sqlite3_errstr(code));
}

