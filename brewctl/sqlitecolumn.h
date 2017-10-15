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
    virtual         ~SQLiteColumn();

    int             index() const { return index_; };
    int             len() const { return len_; };
    const void *    data() const { return data_; };
    bool            isNull() const { return isNull_; };

    int             asInt() const
                    {
                        return (data_ == nullptr) ? 0 : ::strtol(data_, NULL, 10);
                    };

    long            asLong() const
                    {
                        return (data_ == nullptr) ? 0 : ::strtol(data_, NULL, 10);
                    };

    const char *    asCString() const
                    {
                        return (data_ == nullptr) ? "" : data_;
                    };

    double          asDouble() const
                    {
                        return (data_ == nullptr) ? 0.0 : ::strtod(data_, NULL);
                    };

    std::string     asString() const
                    {
                        return std::string((data_ == nullptr) ? "" : data_);
                    }

                    operator int()          const { return asInt();     };
                    operator long()         const { return asLong();    };
                    operator const char *() const { return asCString(); };
                    operator double()       const { return asDouble();  };
                    operator std::string()  const { return asString();  };

protected:
    int             index_;
    char *          data_;
    int             len_;
    bool            isNull_;
};

#endif // SQLITECOLUMN_H_INC

