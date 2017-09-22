#ifndef DATABASE_H_INC
#define DATABASE_H_INC
/*
    database.h: very light wrapper around SQLite, to provide persistent database storage.
    Implemented as a singleton.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "error.h"
#include <string>
#include <memory>

extern "C"
{
#include <sqlite3.h>
}


class Database
{
private:
                            Database();

public:
    virtual                             ~Database();
    static std::unique_ptr<Database>    create(const char * const path, Error& err);
protected:

    sqlite3 *               db_;
    std::string             path_;
};


#endif // DATABASE_H_INC
