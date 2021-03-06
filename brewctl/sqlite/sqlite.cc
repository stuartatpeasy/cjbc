/*
    sqlite.h: minimal wrapper around common SQLite functions and management for a SQLite database object.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/sqlite/sqlite.h"
#include "include/framework/log.h"
#include <cstdlib>
#include <memory>

using std::lock_guard;
using std::mutex;
using std::string;


SQLite::SQLite() noexcept
    : db_(nullptr)
{
}


SQLite::~SQLite() noexcept
{
    close();
}


// open() - attempt to open a new database session using the database file <filename>.
//
bool SQLite::open(const string& filename, const int flags, Error * const err) noexcept
{
    if(!close(err))
        return false;

    logDebug("Opening database file '%s'", filename.c_str());
    const int ret = ::sqlite3_open_v2(filename.c_str(), &db_, flags, NULL);
    if(ret == SQLITE_OK)
    {
        logDebug("Database file '%s' opened successfully", filename.c_str());
        path_ = filename;
        return true;
    }

    fmtErr(err, ret);

    return false;
}


// close() - attempt to close the currently-open database session.  Returns true on success, or if there is no
// currently-open session; false otherwise.
//
bool SQLite::close(Error * const err) noexcept
{
    if(db_ == nullptr)
        return true;        // Database not open - return success

    logDebug("SQLite: closing database");
    const int ret = ::sqlite3_close_v2(db_);
    if(ret == SQLITE_OK)
    {
        db_ = nullptr;
        path_ = "";
        return true;
    }

    fmtErr(err, ret);
    return false;
}


// prepare() - prepare the SQL statement <sql> for execution; on success, return a statement object through <stmt>.
// Return the SQLite retval in all cases.
//
bool SQLite::prepare(const string& sql, SQLiteStmt& stmt, Error * const err) noexcept
{
    int ret;

    if(isOpen())
    {
        lock_guard<mutex> lock(lock_);

        stmt.finalise();        // in case stmt represents an active statement
        ret = ::sqlite3_prepare_v2(db_, sql.c_str(), -1, stmt, NULL);

        if(ret == SQLITE_OK)
        {
            logDebug("SQLite: prepared stmt {%x}: %s", stmt.id(), sql.c_str());
            return true;
        }
    }
    else
    {
        logError("SQLite error: attempted to prepare statement without an open database");
        ret = SQLITE_ABORT;
    }

    logWarning("Failed to prepare stmt: %s", sql.c_str());
    fmtErr(err, ret);
    return false;
}


// prepareAndStep() - prepare the SQL query in <sql> and - if successful - execute step() on the prepared statement in
// order to execute the query and generate the first resulting record.  This method is intended for use with queries in
//which the result-set is not required.
//
bool SQLite::prepareAndStep(const std::string& sql, SQLiteStmt& stmt, Error * const err) noexcept
{
    logDebug("SQLite: prepare-and-step stmt: %s", sql.c_str());
    if(!prepare(sql, stmt, err))
        return false;

    const int ret = stmt.step();
    if(ret == SQLITE_OK)
        return true;

    fmtErr(err, ret);
    return false;
}


// fmtErr() - populate Error object err (if non-null) with the supplied error code and an appropriate
// human-readable error message.
//
void SQLite::fmtErr(Error * const err, const int code) noexcept
{
    logWarning("SQLite error %d: %s", code, ::sqlite3_errstr(code));
    formatError(err, DB_SQLITE_ERROR, ::sqlite3_errstr(code), code);
}

