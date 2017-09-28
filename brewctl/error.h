#ifndef ERROR_H_INC
#define ERROR_H_INC
/*
    error.h: declares the Error class, which acts as a container for runtime errors and their
    descriptions.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include <string>


class Error
{
public:
                    Error();
    virtual         ~Error();

    Error&          format(const int code, const char * const format, ...);
    const std::string&    message() const { return msg_; };
    int             code() const { return code_; };

protected:
    std::string     msg_;
    int             code_;
};

#endif // ERROR_H_INC
