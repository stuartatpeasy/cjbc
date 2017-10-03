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


// open() - open, or create and open, the SQLite database at the specified path.  Returns bool
// indicating success.
//
bool Database::open(const char * const path, Error& err)
{
    return sqlite_.open(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, &err) == SQLITE_OK;
}

