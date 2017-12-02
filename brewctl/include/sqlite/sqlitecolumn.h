#ifndef SQLITE_SQLITECOLUMN_H_INC
#define SQLITE_SQLITECOLUMN_H_INC
/*
    sqlitecolumn.h: abstraction for a column value in a result set returned from a SQLite query

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/stringvalue.h"

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
    SQLiteColumn&           operator=(const SQLiteColumn& rhs) noexcept;
    SQLiteColumn&           operator=(SQLiteColumn&& rhs) noexcept;

    int                     index() const noexcept { return index_; };
    int                     len() const noexcept { return len_; };
    const void *            data() const noexcept { return value_.str().c_str(); };
    bool                    isNull() const noexcept { return isNull_; };

    template<typename T> T  get() const noexcept
                            {
                                return (T) value_;
                            }

private:
    SQLiteColumn&           copy(const SQLiteColumn& rhs) noexcept;
    SQLiteColumn&           move(SQLiteColumn& rhs) noexcept;

    int                     index_;
    StringValue             value_;
    int                     len_;
    bool                    isNull_;
};

#endif // SQLITE_SQLITECOLUMN_H_INC

