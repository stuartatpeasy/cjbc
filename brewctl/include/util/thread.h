#ifndef UTIL_THREAD_H_INC
#define UTIL_THREAD_H_INC
/*
    thread.h: utility functions related to multithreading

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/framework/error.h"
#include <string>


namespace Util::Thread
{
    bool                        setName(const std::string& name) noexcept;
} // namespace Util::Thread

#endif // UTIL_THREAD_H_INC

