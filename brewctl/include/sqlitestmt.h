#ifndef SQLITESTMT_H_INC
#define SQLITESTMT_H_INC
/*
    sqlitestmt.h: wraps a sqlite3_stmt object

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include "sqlitecolumn.h"
#include "error.h"

extern "C"
{
#include <sqlite3.h>
}

typedef std::map<std::string, int> ColNameMap_t;

class SQLiteStmt
{
public:
                        SQLiteStmt() noexcept;
    virtual             ~SQLiteStmt() noexcept;

                        // Indexed bind methods
	bool                bind(const int index, const double arg, Error * const err = nullptr) noexcept;
	bool                bind(const int index, const int arg, Error * const err = nullptr) noexcept;
	bool                bind(const int index, const long long arg, Error * const err = nullptr) noexcept;
	bool                bind(const int index, const std::string& arg, Error * const err = nullptr) noexcept;
	bool                bind(const int index, const int len, const void * const arg, Error * const err = nullptr)
                            noexcept;
	bool                bindNull(const int index, Error * const err = nullptr) noexcept;

                        // Named-parameter bind methods
	bool                bind(const std::string& param, const double arg, Error * const err = nullptr) noexcept;
	bool                bind(const std::string& param, const int arg, Error * const err = nullptr) noexcept;
	bool                bind(const std::string& param, const long long arg, Error * const err = nullptr) noexcept;
	bool                bind(const std::string& param, const std::string& arg, Error * const err = nullptr) noexcept;
	bool                bind(const std::string& param, const int len, const void * const arg,
                             Error * const err = nullptr) noexcept;
	bool                bindNull(const std::string& param, Error * const err = nullptr) noexcept;

	bool                clearBindings(Error * const err = nullptr) noexcept;

    int                 numCols() noexcept;

	bool                step(Error * const err = nullptr) noexcept;
	bool                reset(Error * const err = nullptr) noexcept;
    SQLiteColumn        column(const int index) noexcept;
    SQLiteColumn        column(const std::string& name) noexcept;

                        operator sqlite3_stmt *() noexcept { return stmt_; };
                        operator sqlite3_stmt **() noexcept { return &stmt_; };

    SQLiteColumn        operator[](const int index) noexcept { return column(index); };
    SQLiteColumn        operator[](const std::string& name) noexcept { return column(name); };

    void                finalise() noexcept;
    size_t              id() const noexcept { return (size_t) stmt_; };

private:
	bool                checkError(const int ret, Error * const err, const int successCode = SQLITE_OK) noexcept;
	bool                checkError(const int ret, Error * const err, std::vector<int> successCodes) noexcept;
    void                formatError(Error * const err, const int code) noexcept;
    void                getColumnNames() noexcept;

    sqlite3_stmt *      stmt_;
    bool                firstStepDone_;
    ColNameMap_t        columnNames_;
    static std::mutex   lock_;
};

#endif // SQLITESTMT_H_INC

