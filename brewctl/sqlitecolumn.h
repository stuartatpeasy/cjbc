#ifndef SQLITECOLUMN_H_INC
#define SQLITECOLUMN_H_INC
/*
    sqlitecolumn.h: abstraction for a column value in a result set returned from a SQLite query

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include <string>
#include <cstdlib>      // strtod()

extern "C"
{
#include <sqlite3.h>
}

class SQLiteStmt;


class SQLiteColumn
{
    friend class SQLiteStmt;

protected:
                SQLiteColumn(SQLiteStmt &stmt, const int index);
public:
        virtual ~SQLiteColumn();

            int index() const { return index_; };
            int len() const { return len_; };
   const void * data() const { return data_; };

                operator int();
                operator const char *() { return data_ == nullptr ? "" : (const char *) data_; };
                operator double() { return data_ == nullptr ? 0.0 : ::strtod(data_, NULL); };
                operator std::string() { return data_ == nullptr ? std::string("") : std::string(data_); };

protected:
            int index_;
         char * data_;
            int len_;
};

#endif // SQLITECOLUMN_H_INC

