#ifndef UTIL_THREAD_H_INC
#define UTIL_THREAD_H_INC
/*
    thread.h: utility functions related to multithreading

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/error.h"
#include <string>


namespace Util
{
namespace Thread
{
    bool                        setName(const std::string& name) noexcept;
} // namespace Thread
} // namespace Util

#endif // UTIL_THREAD_H_INC

