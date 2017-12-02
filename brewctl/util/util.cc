/*
    util.cc: utility functions

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/util.h"
#include "include/registry.h"
#include <chrono>
#include <cstdlib>          // NULL
#include <cstring>          // ::strcpy
#include <random>

extern "C"
{
#include <fcntl.h>
#include <netdb.h>
#include <net/if.h>
#include <pthread.h>
#include <pwd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/resource.h>   // ::getrlimit()
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
}

using std::chrono::duration;
using std::chrono::high_resolution_clock;
using std::string;
using std::vector;


namespace Util
{

namespace Net
{

const unsigned short HWADDR_LEN = 6;        // Length, in bytes, of a MAC address

// getInterfaceNames() - return the names of all network interfaces in the system, in the form of a vector<string>.
//
vector<string> getInterfaceNames() noexcept
{
    struct if_nameindex *if_nidxs, *interface;
    vector<string> ret;

    if_nidxs = ::if_nameindex();
    if(if_nidxs != NULL)
    {
        for(interface = if_nidxs; (interface->if_index != 0) || (interface->if_name); ++interface)
            ret.push_back(interface->if_name);
    }

    ::if_freenameindex(if_nidxs);

    return ret;
}

// getInterfaceHardwareAddress() - get the hardware (MAC) address of the specified interface, as a uint64_t.
//
uint64_t getInterfaceHardwareAddress(const string& interface) noexcept
{
    int sock, i;
    struct ifreq ifr;
    uint64_t ret = 0;

    sock = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if(sock < 0)
        return (uint64_t) -1;

    ::strcpy(ifr.ifr_name, interface.c_str());

    if(::ioctl(sock, SIOCGIFHWADDR, &ifr) != 0)
        return (uint64_t) -1;

    for(i = 0; i < HWADDR_LEN; ++i)
        ret = (ret << 8) + (unsigned char) (ifr.ifr_hwaddr.sa_data)[i];

    ::close(sock);

    return ret;
}


// getRandomHardwareAddress() - get a random hardware (MAC) address as a uint64_t.
//
uint64_t getRandomHardwareAddress() noexcept
{
    uint64_t ret = Util::Random::randomUnsignedInt();
    ret = (ret << 32) + Util::Random::randomUnsignedInt();

    return ret & 0xffffffffffff;
}

} // namespace Net

namespace Random
{

static unsigned int seedVal_ = 0;


// seed() - seed the random-number generator with the specified <seedVal>.
//
void seed(unsigned int seedVal) noexcept
{
    seedVal_ = seedVal;
}


// seed() - seed the random-number generator with the current time from the high-resolution clock
//
void seed() noexcept
{
    seed(high_resolution_clock::now().time_since_epoch().count());
}


// randomUnsignedInt() - return a random unsigned integer.
//
unsigned int randomUnsignedInt() noexcept
{
    std::minstd_rand0 generator(seedVal_);
    unsigned int ret = generator();

    seedVal_ = ret;

    return ret;
}

} // namespace Random

namespace Sys
{

// checkSyscall() - local helper function.  Takes the return value from a syscall in <ret>, compares it with a value
// indicating failure (in <errVal>); if the two values are equal (i.e. the syscall failed), format an appropriate error
// message in <err>, using the SYSCALL_FAILED error code and appending an error string and the value of errno.  Returns
// false if <ret> == <errVal> (i.e. the syscall failed); true otherwise.
// 
static bool checkSyscall(const int ret, const string& name, Error * const err, const int errVal = -1) noexcept
{
    if(ret == errVal)
    {
        formatErrorWithErrno(err, SYSCALL_FAILED, name);
        return false;
    }

    return true;
}

// daemonise() - convert the current foreground process to a daemon using the procedure described in daemon(7).
// Drop privileges by switching to the user specified by <user>.  Returns true on success; false otherwise.
//
bool daemonise(const string& user, Error * const err) noexcept
{
    // If <user> is non-empty, validate username by attempting to map it to a uid.
    bool switchUser = false;
    uid_t uid;
    if(user.length())
    {
        errno = 0;
        struct passwd *pw = ::getpwnam(user.c_str());
        if(pw == NULL)
        {
            if(!errno)
                formatError(err, NO_SUCH_USER, user);                       // user not found
            else
                formatErrorWithErrno(err, SYSCALL_FAILED, "getpwnam()");    // getpwnam() failed for some other reason

            return false;
        }

        uid = pw->pw_uid;
        switchUser = true;
    }

    // 1. Ensure that all open files (except stdin, stdout and stderr) are closed.
    struct rlimit rlim;
    if(!checkSyscall(::getrlimit(RLIMIT_NOFILE, &rlim), "getrlimit(RLIMIT_NOFILE)", err))
        return false;

    for(int fd = rlim.rlim_max - 1; fd >= 3; --fd)
        ::close(fd);

    // 2. Reset all signal handlers to their default.
    for(int signum = 0; signum < _NSIG; ++signum)
    {
        if(!checkSyscall(::signal(signum, SIG_DFL) == SIG_ERR, "signal()", err, 0))
            return false;
    }

    // 3. Reset the signal mask.
    sigset_t sigset;
    if(!checkSyscall(::sigemptyset(&sigset), "sigemptyset()", err) ||
       !checkSyscall(::sigprocmask(SIG_SETMASK, &sigset, NULL), "sigprocmask()", err))
        return false;

    // 4. Sanitize the environment block.
    // (this is currently a no-op)

    // 5. Call fork() to create a background process.
    pid_t pid;
    if(!checkSyscall(pid = ::fork(), "fork()", err))
        return false;

    if(!pid)        // The following block executes in the new child's process.
    {
        // 6. Call setsid() to detach from any terminal and create an independent session.
        if(!checkSyscall((int) ::setsid(), "setsid()", err))
            return false;

        // 7. fork() again, to ensure that the daemon can never re-acquire a terminal.
        if(!checkSyscall(pid = ::fork(), "fork()", err))
            return false;

        // 8. Call exit() in the first child (i.e. the second child's parent) to re-parent the second child to init
        // (pid 1).
        if(pid)
            ::exit(0);      // Exit the parent process

        // 9. Connect /dev/null to stdin, stdout and stderr.
        int fd;
        if(!checkSyscall(fd = ::open("/dev/null", O_RDWR), "open()", err)   ||
           !checkSyscall(::dup2(fd, 0), "dup2()", err)                      ||
           !checkSyscall(::dup2(fd, 1), "dup2()", err)                      ||
           !checkSyscall(::dup2(fd, 2), "dup2()", err)                      ||
           !checkSyscall(::close(fd), "close()", err))
            return false;

        // 10. Reset umask to 0, so that file modes passed to ::open(), ::mkdir() etc directly control the access mode
        // of the created files and directories.
        umask(0);       // Always succeeds

        // 11. Change the current working directory to the root dir (/), so that the daemon does not block the
        // unmounting of mount points.
        if(!checkSyscall(::chdir("/"), "chdir()", err))
            return false;

        // 12. Write the daemon's PID to pid file to ensure that the daemon cannot be started more than once
        // TODO

        // 13. Drop privileges, if required
        if(switchUser)
        {
            if(!checkSyscall(::setuid(uid), "setuid()", err))
                return false;
        }

        // 14. From the daemon process, notify the original parent process that initialisation is complete
        // (this is currently a no-op)
    }
    else
        ::exit(0);      // 15. Call ::exit() in the original parent process.

    return true;        // FIXME
}

} // namespace Sys

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

