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
    DB_TOO_FEW_COLUMNS = 0x10000000,
    UNKNOWN_ERROR      = 0xffffffff
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

    bool                format(const int code, const char * const format, ...);
    bool                format(const ErrorCode_t code, ...);
    const std::string&  message() const { return msg_; };
    int                 code() const { return code_; };

private:
    Error&              init(const Error& rhs);
    void                vformat(const int code, const char * const format, va_list args);

    std::string         msg_;
    int                 code_;
};

#endif // ERROR_H_INC
