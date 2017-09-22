/*
    error.h: declares the Error class, which acts as a container for runtime errors and their
    descriptions.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "error.h"
#include <cstdarg>
#include <cstdio>


static const int msg_buffer_len = 4096; // Length of buffer in which the error message is written

Error::Error()
    : msg_(""), code_(0)
{

}


Error::~Error()
{
}


Error& Error::format(const int code, const char * const format, ...)
{
    va_list ap;
    char buffer[msg_buffer_len];

    va_start(ap, format);

    const int ret = ::vsnprintf(buffer, msg_buffer_len, format, ap);
    if(ret >= msg_buffer_len)
        buffer[msg_buffer_len - 1] = '\0';      // Output truncated

    code_ = code;
    msg_ = buffer;

    return *this;
}

