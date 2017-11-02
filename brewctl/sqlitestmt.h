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
                    SQLiteStmt();
    virtual         ~SQLiteStmt();

                    // Indexed bind methods
	bool            bind(const int index, const double arg, Error * const err = nullptr);
	bool            bind(const int index, const int arg, Error * const err = nullptr);
	bool            bind(const int index, const long long arg, Error * const err = nullptr);
	bool            bind(const int index, const std::string& arg, Error * const err = nullptr);
	bool            bind(const int index, const int len, const void * const arg, Error * const err = nullptr);
	bool            bindNull(const int index, Error * const err = nullptr);

                    // Named-parameter bind methods
	bool            bind(const std::string& param, const double arg, Error * const err = nullptr);
	bool            bind(const std::string& param, const int arg, Error * const err = nullptr);
	bool            bind(const std::string& param, const long long arg, Error * const err = nullptr);
	bool            bind(const std::string& param, const std::string& arg, Error * const err = nullptr);
	bool            bind(const std::string& param, const int len, const void * const arg, Error * const err = nullptr);
	bool            bindNull(const std::string& param, Error * const err = nullptr);

	bool            clearBindings(Error * const err = nullptr);

    int             numCols();

	bool            step(Error * const err = nullptr);
	bool            reset(Error * const err = nullptr);
    SQLiteColumn    column(const int index);
    SQLiteColumn    column(const std::string& name);

                    operator sqlite3_stmt *() { return stmt_; };
                    operator sqlite3_stmt **() { return &stmt_; };

    SQLiteColumn    operator[](const int index) { return column(index); };
    SQLiteColumn    operator[](const std::string& name) { return column(name); };

    void            finalise();
    size_t          id() const { return (size_t) stmt_; };

private:
	bool            checkError(const int ret, Error * const err, const int successCode = SQLITE_OK);
	bool            checkError(const int ret, Error * const err, std::vector<int> successCodes);
    void            formatError(Error * const err, const int code);
    void            getColumnNames();

    sqlite3_stmt *  stmt_;
    bool            firstStepDone_;
    ColNameMap_t    columnNames_;
};

#endif // SQLITESTMT_H_INC

