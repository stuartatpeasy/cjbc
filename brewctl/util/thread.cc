/*
    thread.cc: utility functions relating to multithreading

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/util/thread.h"

extern "C"
{
#include <pthread.h>
}

using std::string;


namespace Util
{
namespace Thread
{

// setName() - set the current thread's name/title, as reported by ps, top, etc.  Returns true on success, or false if
// the length of <name> exceeds the limit specified by the operating system.
//
bool setName(const string& name) noexcept
{
    return ::pthread_setname_np(::pthread_self(), name.c_str());
}

} // namespace Thread
} // namespace Util

