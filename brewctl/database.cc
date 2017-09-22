/*
    database.cc: very light wrapper around SQLite, to provide persistent database storage.
    Implemented as a singleton.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "database.h"
#include <cerrno>
#include <cstdlib>

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

using std::string;
using std::unique_ptr;


Database::Database()
{
}


Database::~Database()
{
}


// create() - open, or create, the SQLite database at the specified path.  Returns a ptr to a
// Database object on success, or NULL on failure.
//
unique_ptr<Database> Database::create(const char * const path, Error& err)
{
    struct stat st;
    const int errno_old = errno;

    if(!::stat(path, &st))
    {
        // Success - path exists
        if(S_ISDIR(st.st_mode))
        {
            err.format(EISDIR, "Failed to open database file '%s': is a directory", path);
            return NULL;
        }

        auto db = std::make_unique<Database>();

        int ret = ::sqlite3_open(path, &db.db_);
        if(ret == SQLITE_OK)
            return db;

        err.format(ret, ::sqlite3_errstr(ret));
        return NULL;
    }

    //
    if(errno == ENOENT)
    {
        // A component of the path doesn't exist.  Attempt to create the file.
    }
    else
    {
    }

        return false;


}

