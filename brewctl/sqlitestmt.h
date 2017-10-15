#ifndef SQLITESTMT_H_INC
#define SQLITESTMT_H_INC
/*
    sqlitestmt.h: wraps a sqlite3_stmt object

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include <string>
#include <memory>
#include "sqlitecolumn.h"
#include "error.h"

extern "C"
{
#include <sqlite3.h>
}


class SQLiteStmt
{
public:
                                    SQLiteStmt();
                            virtual ~SQLiteStmt();

                                    // Indexed bind methods
                               bool bind(const int index, const double arg,
                                         Error * const err = nullptr);
                               bool bind(const int index, const int arg,
                                         Error * const err = nullptr);
                               bool bind(const int index, const long long arg,
                                         Error * const err = nullptr);
                               bool bind(const int index, const std::string& arg,
                                         Error * const err = nullptr);
                               bool bind(const int index, const int len, const void * const arg,
                                         Error * const err = nullptr);
                               bool bindNull(const int index, Error * const err = nullptr);

                                    // Named-parameter bind methods
                               bool bind(const std::string& param, const double arg,
                                         Error * const err = nullptr);
                               bool bind(const std::string& param, const int arg,
                                         Error * const err = nullptr);
                               bool bind(const std::string& param, const long long arg,
                                         Error * const err = nullptr);
                               bool bind(const std::string& param, const std::string& arg,
                                         Error * const err = nullptr);
                               bool bind(const std::string& param, const int len,
                                         const void * const arg, Error * const err = nullptr);
                               bool bindNull(const std::string& param, Error * const err = nullptr);

                               bool clearBindings(Error * const err = nullptr);

                                int numCols();

                               bool step(Error * const err = nullptr);
                               bool reset(Error * const err = nullptr);
      std::unique_ptr<SQLiteColumn> column(const int index);

                                    operator sqlite3_stmt *() { return stmt_; };
                                    operator sqlite3_stmt **() { return &stmt_; };

private:
                               bool checkError(const int ret, Error * const err,
                                               int successCode = SQLITE_OK);
                               void formatError(Error * const err, const int code);

                     sqlite3_stmt * stmt_;
};

#endif // SQLITESTMT_H_INC

