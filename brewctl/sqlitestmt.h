#ifndef SQLITESTMT_H_INC
#define SQLITESTMT_H_INC
/*
    sqlitestmt.h: wraps a sqlite3_stmt object

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include <string>

extern "C"
{
#include <sqlite3.h>
}


class SQLiteStmt
{
public:
                        SQLiteStmt();
    virtual             ~SQLiteStmt();

    int                 bind(const int index, const double arg);
    int                 bind(const int index, const int arg);
    int                 bind(const int index, const long long arg);
    int                 bind(const int index, const std::string& arg);
    int                 bind(const int index, const char * const arg);
    int                 bind(const int index, const int len, const void * const arg);
    int                 bindNull(const int index);

    int                 clearBindings();

    int                 step();
    int                 reset();

                        operator sqlite3_stmt *() { return stmt_; };
                        operator sqlite3_stmt **() { return &stmt_; };

private:
    sqlite3_stmt *      stmt_;
};


#endif // SQLITESTMT_H_INC

