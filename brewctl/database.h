#ifndef DATABASE_H_INC
#define DATABASE_H_INC
/*
    database.h: very light wrapper around SQLite, to provide persistent database storage.
    Implemented as a singleton.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

extern "C"
{
#include <sqlite3.h>
};


class Database
{
protected:
                            Database();
    virtual                 ~Database();
    bool                    init();

public:
    Database *              instance();

protected:
    sqlite3 *               db_;
};


#endif // DATABASE_H_INC
