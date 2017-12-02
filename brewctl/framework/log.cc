/*
    log.cc: global logging functions

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "include/framework/log.h"
#include <boost/algorithm/string.hpp>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

extern "C"
{
#include <syslog.h>
}

using std::string;
using std::ostringstream;

static const size_t LOG_BUF_SIZE = 16384;

static FILE *logFp = NULL;
static LogMethod_t logMethod = LOG_METHOD_NONE;

static string logLevelStr(const LogLevel_t level) noexcept;
static bool logWrite(const LogLevel_t level, const string& str) noexcept;
static int logLevelToSyslogLevel(const LogLevel_t level) noexcept;
LogLevel_t logLevelFromStr(const string& level) noexcept;

static LogLevel_t logLevel = LOG_LEVEL_DEBUG;


// logInit() - initialise the logging system according to the method string in <method>
//
bool logInit(const string& method) noexcept
{
    if(method == "syslog")
    {
        logClose();
        ::openlog(NULL, LOG_CONS | LOG_PID, LOG_DAEMON);
        logMethod = LOG_METHOD_SYSLOG;

        return true;
    }
    else if(method.substr(0, 5) == "file:")
    {
        FILE *fpNew;

        fpNew = ::fopen(method.substr(5).c_str(), "a");
        if(fpNew == NULL)
            return false;

        logClose();

        logFp = fpNew;
        logMethod = LOG_METHOD_FILE;

        return true;
    }

    return false;
}


// logClose() - close the log, and set logMethod to LOG_METHOD_NONE.
//
void logClose() noexcept
{
    switch(logMethod)
    {
        case LOG_METHOD_NONE:
            break;

        case LOG_METHOD_FILE:
            if(logFp != NULL)
            {
                ::fclose(logFp);
                logFp = NULL;
            }
            break;

        case LOG_METHOD_SYSLOG:
            ::closelog();
            break;
    }

    logMethod = LOG_METHOD_NONE;
}


// logSetLevel() - set the current logging level
//
LogLevel_t logSetLevel(const LogLevel_t newLevel) noexcept
{
    const LogLevel_t oldLevel = logLevel;

    if(newLevel < LOG_LEVEL_INVALID)
        logLevel = newLevel;

    return oldLevel;
}


// logSetLevel() - set the current logging level using the level string in <newLevel>.
//
LogLevel_t logSetLevel(const string& newLevel) noexcept
{
    return logSetLevel(logLevelFromStr(newLevel));
}


// logGetLevel() - get the current logging level
//
LogLevel_t logGetLevel() noexcept
{
    return logLevel;
}


// logWrite() - write the contents of <str> to the current log destination.
//
static bool logWrite(const LogLevel_t level, const string& str) noexcept
{
    switch(logMethod)
    {
        case LOG_METHOD_FILE:
            return (logFp == NULL) ?  false : (::fwrite(str.c_str(), 1, str.length(), logFp) == str.length());

        case LOG_METHOD_SYSLOG:
            ::syslog(logLevelToSyslogLevel(level), str.c_str());
            return true;

        default:    // Fall through
        case LOG_METHOD_NONE:
            return false;
    }
}


// logLevelStr() - return a ptr to a string describing the log level specified in <level>.
//
static string logLevelStr(const LogLevel_t level) noexcept
{
    switch(level)
    {
        case LOG_LEVEL_DEBUG:   return "DEBUG";
        case LOG_LEVEL_INFO:    return "INFO";
        case LOG_LEVEL_NOTICE:  return "NOTICE";
        case LOG_LEVEL_WARNING: return "WARNING";
        case LOG_LEVEL_ERROR:   return "ERROR";
        default:                return "???";
    }
}


// logLevelFromStr() - return a LogLevel_t corresponding to the string in <level>.  Uses case-insensitive comparison.
//
LogLevel_t logLevelFromStr(const string& level) noexcept
{
         if(boost::iequals(level, "DEBUG"))     return LOG_LEVEL_DEBUG;
    else if(boost::iequals(level, "INFO"))      return LOG_LEVEL_INFO;
    else if(boost::iequals(level, "NOTICE"))    return LOG_LEVEL_NOTICE;
    else if(boost::iequals(level, "WARNING"))   return LOG_LEVEL_WARNING;
    else if(boost::iequals(level, "ERROR"))     return LOG_LEVEL_ERROR;
    else                                        return LOG_LEVEL_INVALID;
}


// logLevelToSyslogLevel() - given a LogLevel_t in <level>, return a corresponding value for the <level> argument to
// ::syslog().
//
static int logLevelToSyslogLevel(const LogLevel_t level) noexcept
{
    switch(level)
    {
        case LOG_LEVEL_DEBUG:   return LOG_DEBUG;
        case LOG_LEVEL_INFO:    return LOG_INFO;
        case LOG_LEVEL_NOTICE:  return LOG_NOTICE;
        case LOG_LEVEL_WARNING: return LOG_WARNING;
        default:    /* Fall through */
        case LOG_LEVEL_ERROR:   return LOG_ERR;
    }
}


// doLog() - called by the log*() macros, this function does the actual logging.
//
bool doLog(const char * const file, const int line, const LogLevel_t level, const std::string& fmt, ...) noexcept
{
    if(level < logLevel)
        return true;

    va_list ap;
    va_start(ap, fmt);

    return doLogV(file, line, level, fmt, ap);
}


// doLogV() - called by the log*V() macros, this function does the actual logging.
//
bool doLogV(const char * const file, const int line, const LogLevel_t level, const std::string& fmt, va_list args)
        noexcept
{
    char logBuf[LOG_BUF_SIZE];
    ostringstream msg;

    if(::vsnprintf(logBuf, LOG_BUF_SIZE, fmt.c_str(), args) < 0)
        return false;

    msg << "<" << file << " +" << line << "> [" << logLevelStr(level) << "] " << logBuf;

    return logWrite(level, msg.str());
}

