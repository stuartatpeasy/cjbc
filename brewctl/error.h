#ifndef ERROR_H_INC
#define ERROR_H_INC
/*
    error.h: declares the Error class, which acts as a container for runtime errors and their
    descriptions.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include <string>


typedef enum
{
    MISSING_ARGVAL          = 0x00000001,
    UNKNOWN_ARG             = 0x00000002,
    CFG_FILE_OPEN_FAILED    = 0x00000003,
    MALLOC_FAILED           = 0x10000000,
    DB_OPEN_FAILED          = 0x11000000,
    DB_TOO_FEW_COLUMNS      = 0x11000001,
    DB_SQLITE_ERROR         = 0x11000002,
    DB_SQLITESTMT_ERROR     = 0x11000003,
    SPI_MODE_SET_FAILED     = 0x12000000,
    SPI_DEVICE_OPEN_FAILED  = 0x12000001,
    SPI_PARAM_SET_FAILED    = 0x12000002,
    UNKNOWN_ERROR           = 0xffffffff
} ErrorCode_t;


class Error
{
public:
                        Error();
                        Error(const Error& rhs);
                        Error(Error&& rhs) noexcept;
    virtual             ~Error();

    Error&              operator=(const Error& rhs);
    Error&              operator=(Error&& rhs) noexcept;

    bool                format(const ErrorCode_t code, ...);
    void                formatV(const ErrorCode_t code, const std::string& format, va_list args);
    static std::string  stringFromCode(const ErrorCode_t code);
    const std::string&  message() const { return msg_; };
    int                 code() const { return code_; };

private:
    Error&              init(const Error& rhs);

    std::string         msg_;
    int                 code_;
};

#endif // ERROR_H_INC
