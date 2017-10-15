/*
    log.cc: global logging functions
*/

#include "log.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C"
{
#include <syslog.h>
}

#define LOG_BUF_SIZE (16384)

static FILE *logFp = NULL;
static LogMethod_t logMethod = LOG_METHOD_NONE;

static const char * logLevelStr(const LogLevel_t level);
static bool logWrite(const LogLevel_t level, const char * const str);
static int logLevelToSyslogLevel(const LogLevel_t level);


// logInit() - initialise the logging system according to the method string in <method>
//
bool logInit(const char * method)
{
    if(!::strcmp(method, "syslog"))
    {
        logClose();
        ::openlog(NULL, LOG_CONS | LOG_PID, LOG_DAEMON);
        logMethod = LOG_METHOD_SYSLOG;

        return true;
    }
    else if(!::strncmp(method, "file:", 5))
    {
        FILE *fpNew;

        method += 5;    // Advance <method> past the "file:" prefix

        fpNew = ::fopen(method, "a");
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
void logClose()
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


// logWrite() - write the contents of <str> to the current log destination.
//
static bool logWrite(const LogLevel_t level, const char * const str)
{
    const size_t len = ::strlen(str);

    switch(logMethod)
    {
        case LOG_METHOD_FILE:
            return (logFp == NULL) ? false : (::fwrite(str, 1, len, logFp) == len);

        case LOG_METHOD_SYSLOG:
            ::syslog(logLevelToSyslogLevel(level), str);
            return true;

        default:    // Fall through
        case LOG_METHOD_NONE:
            return false;
    }
}


// logLevelStr() - return a ptr to a string describing the log level specified in <level>.
//
static const char * logLevelStr(const LogLevel_t level)
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


// logLevelToSyslogLevel() - given a LogLevel_t in <level>, return a corresponding value for the
// <level> argument to ::syslog().
//
static int logLevelToSyslogLevel(const LogLevel_t level)
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
bool doLog(const char * const file, const int line, const LogLevel_t level, const char * const fmt,
           ...)
{
    va_list ap;
    va_start(ap, fmt);
    char logBuf[LOG_BUF_SIZE];
    int bytesRemaining = LOG_BUF_SIZE;

    const int prefixLen = ::snprintf(logBuf, bytesRemaining, "<%s +%d> [%s] ", file, line,
                                     logLevelStr(level));
    if((prefixLen < 0) || (prefixLen >= bytesRemaining))
        return false;

    bytesRemaining -= prefixLen;

    const int ret = ::vsnprintf(logBuf + prefixLen, bytesRemaining, fmt, ap);
    if((ret < 0) || (ret >= bytesRemaining))
        return false;

    return logWrite(level, logBuf);
}

