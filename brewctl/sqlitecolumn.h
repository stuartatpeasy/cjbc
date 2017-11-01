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
                    SQLiteColumn(SQLiteStmt &stmt, const int index) noexcept;
                    SQLiteColumn(SQLiteColumn&& rhs) noexcept;
                    SQLiteColumn(const SQLiteColumn& rhs) noexcept;
public:
    virtual         ~SQLiteColumn() noexcept;

    int             index() const noexcept { return index_; };
    int             len() const noexcept { return len_; };
    const void *    data() const noexcept { return data_; };
    bool            isNull() const noexcept { return isNull_; };

    int             asInt() const noexcept
                    {
                        return (data_ == nullptr) ? 0 : ::strtol(data_, NULL, 10);
                    };

    long            asLong() const noexcept
                    {
                        return (data_ == nullptr) ? 0 : ::strtol(data_, NULL, 10);
                    };

    const char *    asCString() const noexcept
                    {
                        return (data_ == nullptr) ? "" : data_;
                    };

    double          asDouble() const noexcept
                    {
                        return (data_ == nullptr) ? 0.0 : ::strtod(data_, NULL);
                    };

    std::string     asString() const noexcept
                    {
                        return std::string((data_ == nullptr) ? "" : data_);
                    }

                    operator int()          const noexcept { return asInt();     };
                    operator long()         const noexcept { return asLong();    };
                    operator const char *() const noexcept { return asCString(); };
                    operator double()       const noexcept { return asDouble();  };
                    operator std::string()  const noexcept { return asString();  };

    SQLiteColumn&   operator=(const SQLiteColumn& rhs) noexcept;
    SQLiteColumn&   operator=(SQLiteColumn&& rhs) noexcept;

private:
    SQLiteColumn&   copy(const SQLiteColumn& rhs) noexcept;
    SQLiteColumn&   move(SQLiteColumn& rhs) noexcept;

    int             index_;
    char *          data_;
    int             len_;
    bool            isNull_;
};

#endif // SQLITECOLUMN_H_INC

