/*
    error.h: declares the Error class, which acts as a container for runtime errors and their
    descriptions.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "error.h"
#include <cstdarg>
#include <cstdio>
#include <map>

using std::string;
using std::map;


static map<ErrorCode_t, const char *> errorMessages =
{
    {DB_TOO_FEW_COLUMNS,        "Database error: too few columns returned by query"}
};



static const int msg_buffer_len = 4096; // Length of buffer in which the error message is written

Error::Error()
    : msg_(""), code_(0)
{

}


Error::Error(const Error& rhs)
{
    init(rhs);
}


Error::Error(Error&& rhs) noexcept
{
    init(rhs);
    rhs.msg_ = string("");
    rhs.code_ = 0;
}


Error::~Error()
{
}


Error& Error::operator=(const Error& rhs)
{
    return init(rhs);
}


Error& Error::operator=(Error&& rhs) noexcept
{
    init(rhs);
    rhs.msg_ = string("");
    rhs.code_ = 0;

    return *this;
}


// init() - helper function used by copy ctor and assignment operator to initialise this object
// from another Error object.
//
Error& Error::init(const Error& rhs)
{
    msg_ = rhs.msg_;
    code_ = rhs.code_;

    return *this;
}


// format() - format and store the error message specified by <format>; store <code> as an error
// code.  Always returns false, so that a call to this method can be used as a retval in a failing
// bool-returning method.
//
bool Error::format(const int code, const char * const format, ...)
{
    va_list ap;
    va_start(ap, format);

    vformat(code, format, ap);
    return false;
}


// format() - format and store the error message identified by <code>.  Always returns false, so
// that a call to this method can be used as a retval in a failing bool-returning method.
//
bool Error::format(const ErrorCode_t code, ...)
{
    va_list ap;
    va_start(ap, code);
    const char *fmtstr;

    auto msg = errorMessages.find(code);
    fmtstr = (msg == errorMessages.end()) ? "Unknown internal error" : msg->second;

    vformat(code, fmtstr, ap);
    return false;
}


// vformat() - [private] use <args> to fill in the format string <format>, and store it; store
// <code>.
//
void Error::vformat(const int code, const char * const format, va_list args)
{
    char buffer[msg_buffer_len];

    const int ret = ::vsnprintf(buffer, msg_buffer_len, format, args);
    if(ret >= msg_buffer_len)
        buffer[msg_buffer_len - 1] = '\0';      // Output truncated

    code_ = code;
    msg_ = buffer;
}

