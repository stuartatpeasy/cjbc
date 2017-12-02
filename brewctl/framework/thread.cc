/*
    thread.cc: interface defining a class intended to run in its own thread

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/framework/thread.h"
#include "include/framework/registry.h"
#include "include/util/thread.h"


// ctor - mark thread as not-running and not-requested-to-stop.
//
Thread::Thread() noexcept
    : stop_(false), running_(false)
{
}


// stop() - called when the thread's owner wishes to stop the thread.  Sets the <stop_> member to true; the derived
// class should monitor this member to decide when to cleanly stop operating.
//
void Thread::stop() noexcept
{
    stop_ = true;
}


// setName() - set the thread's name, prepending the application's short name; e.g. "<app_short_name>: <name>".
// The maximum length of the thread's name is 16 characters, including the terminating NUL character.  This method
// returns true if the resulting name is within the allowable length; false otherwise.
//
bool Thread::setName(const std::string& name) noexcept
{
    return Util::Thread::setName(Registry::instance().config()("application.short_name") + ": " + name);
}

