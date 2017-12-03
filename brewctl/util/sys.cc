/*
    sys.cc: utility functions related to system operations

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/util/sys.h"
#include "include/framework/log.h"
#include "include/framework/registry.h"
#include "include/util/string.h"
#include <boost/regex.hpp>
#include <cstdlib>          // NULL

extern "C"
{
#include <fcntl.h>
#include <pthread.h>
#include <pwd.h>
#include <signal.h>
#include <sys/file.h>       // ::flock()
#include <sys/resource.h>   // ::getrlimit()
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
}

using boost::regex;
using std::stol;
using std::string;


namespace Util::Sys
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
        formatErrorWithErrno(err, SYSCALL_FAILED, name.c_str());
        return false;
    }

    return true;
}


// daemonise() - convert the current foreground process to a daemon using the procedure described in daemon(7).
// Drop privileges by switching to the user specified by <user>.  Returns true on success; false otherwise.
//
bool daemonise(Error * const err) noexcept
{
    // 1. Ensure that all open files (except stdin, stdout and stderr) are closed.
    struct rlimit rlim;
    if(!checkSyscall(::getrlimit(RLIMIT_NOFILE, &rlim), "getrlimit(RLIMIT_NOFILE)", err))
        return false;

    for(int fd = rlim.rlim_max - 1; fd >= 3; --fd)
        ::close(fd);

    // 2. Reset all signal handlers to their default.
    struct sigaction sa;
    sigset_t blockmask;

    if(!checkSyscall(::sigemptyset(&blockmask), "sigemptyset()", err))
        return false;

    for(int signum = 1; signum < _NSIG; ++signum)
    {
        errno = 0;
        sa.sa_handler = SIG_DFL;
        sa.sa_sigaction = NULL;
        sa.sa_mask = blockmask;
        sa.sa_flags = 0;

        if((::sigaction(signum, &sa, NULL) == -1) && (errno != EINVAL))
        {
            formatErrorWithErrno(err, SYSCALL_FAILED, "sigaction()");
            return false;
        }
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
        // This isn't done here, but is instead done separately by the caller.  That way, it is easier for the
        // application to drop privileges without daemonising, thereby simplifying debugging.

        // 14. From the daemon process, notify the original parent process that initialisation is complete
        // (this is currently a no-op)
    }
    else
        ::exit(0);      // 15. Call ::exit() in the original parent process.

    return true;
}


// getUid() - given a username, or a numeric user ID, in <username>, return the corresponding user ID.  If the user does
// not exist, return (uid_t) -1.  If an error occurs, return (uid_t) -1 and set <err> accordingly.
//
uid_t getUid(const string& username, Error * const err) noexcept
{
    int intUid;
    if(Util::String::isIntStr(username, &intUid))
    {
        if(intUid >= 0)
            return (uid_t) intUid;

        formatError(err, INVALID_USER_ID, intUid);
        return (uid_t) -1;
    }

    errno = 0;
    struct passwd *pw = ::getpwnam(username.c_str());
    if(pw == NULL)
    {
        if(errno)
            formatErrorWithErrno(err, SYSCALL_FAILED, "getpwnam()");

        formatError(err, NO_SUCH_USER, username.c_str());
        return (uid_t) -1;
    }

    return pw->pw_uid;
}


// setUid() - given a username, or a numeric user ID, in <username>, change the user ID of the current process to the
// specified user.  Returns true on success; false otherwise.
//
bool setUid(const string& username, Error * const err) noexcept
{
    const uid_t uid = getUid(username, err);
    if(uid == (uid_t) -1)
        return false;           // No such user, or username lookup failed

    const bool ret = checkSyscall(::setuid(uid), "setuid()", err);
    if(ret)
        logInfo("Changed to user '%s' (%d)", username.c_str(), uid);

    return ret;
}


// readPidFile() - read a PID from the file specified in <filename> and return it.  Return -1 if the file could not be
// read, or its contents is not a valid PID.
//
int readPidFile(const string& filename, Error * const err) noexcept
{
    int fd, nread;
    char buffer[64];

    errno = 0;
    fd = ::open(filename.c_str(), O_RDONLY);
    if(fd == -1)
    {
        // If the file was not found, return -1 without setting an error in <err>.  Otherwise, report the error through
        // <err>.
        if(errno != ENOENT)
            formatError(err, SYSCALL_FAILED, "open()");

        return -1;
    }

    if(!checkSyscall(nread = ::read(fd, buffer, sizeof(buffer) - 1), "read()", err))
    {
        ::close(fd);
        return -1;
    }

    ::close(fd);
    buffer[nread] = '\0';
    string pidstr = buffer;

    if(nread && Util::String::isIntStr(pidstr))
    {
        int pidval = ::stol(pidstr);
        if(pidval > 0)
            return pidval;
    }

    formatError(err, CORRUPT_PIDFILE, filename.c_str());
    return -1;
}


// writePidFile() - write the current process ID to the file specified by <filename>, after first verifying that the
// file does not already contain the PID of a running process.  Returns true on success; false otherwise.
//
bool writePidFile(const string& filename, Error * const err) noexcept
{
    int fd, nread;
    char buffer[64];

    // Attempt to open or create the PID file
    errno = 0;
    fd = ::open(filename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(!checkSyscall(fd, "open()", err))
        return false;

    // Read the contents of the file; if it is non-empty, it should contain an integer PID.
    if(!checkSyscall(nread = ::read(fd, buffer, sizeof(buffer) - 1), "read()", err))
    {
        ::close(fd);
        return false;
    }

    buffer[nread] = '\0';
    string pidstr = buffer;

    if(nread)
    {
        if(Util::String::isIntStr(pidstr))
        {
            int pidval = ::stol(pidstr);
            if(pidval > 0)
            {
                // Found a valid PID in the PID file; see whether it represents a running process.
                errno = 0;
                if(::kill(pidval, 0) == 0)
                {
                    formatError(err, ALREADY_RUNNING);
                    ::close(fd);
                    return false;
                }
                else if(errno != ESRCH)
                {
                    formatErrorWithErrno(err, SYSCALL_FAILED, "kill()");
                    ::close(fd);
                    return false;
                }
                else
                    logInfo("Truncating PID file containing stale process ID %d", pidval);
            }
        }
        else
            logWarning("PID file '%s' seems to be corrupt (contents '%s'); truncating it", filename.c_str(), pidstr);

        // Found data in the PID file, but it does not represent a running process.  Truncate the PID file.
        if(!checkSyscall(::ftruncate(fd, 0), "ftruncate()", err) ||
           !checkSyscall(::lseek(fd, 0, SEEK_SET), "lseek()", err))
        {
            ::close(fd);
            return false;
        }
    }

    // At this point, we can be confident that the PID file is open and empty, and we are the only running process of
    // our type.
    if(!checkSyscall(::sprintf(buffer, "%d", ::getpid()), "sprintf()", err))
        return false;

    int len = ::strlen(buffer);
    const int ret = ::write(fd, buffer, len);

    if(ret == -1)
    {
        formatErrorWithErrno(err, SYSCALL_FAILED, "write()");
        ::close(fd);
        return false;
    }

    if(ret != len)
    {
        formatError(err, INCOMPLETE_WRITE, filename.c_str(), len, ret);
        ::close(fd);
        return false;
    }

    return checkSyscall(::close(fd), "close()", err);
}

} // namespace Util::Sys

