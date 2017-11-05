#ifndef LOG_H_INC
#define LOG_H_INC
/*
    log.h: global logging functions

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include <string>


typedef enum
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_NOTICE,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_INVALID           // Not a real log level - just a placeholder
} LogLevel_t;

typedef enum
{
    LOG_METHOD_NONE = 0,
    LOG_METHOD_FILE,
    LOG_METHOD_SYSLOG
} LogMethod_t;

#define logDebug(...)       doLog(__FILE__, __LINE__, LOG_LEVEL_DEBUG, __VA_ARGS__)
#define logInfo(...)        doLog(__FILE__, __LINE__, LOG_LEVEL_INFO, __VA_ARGS__)
#define logNotice(...)      doLog(__FILE__, __LINE__, LOG_LEVEL_NOTICE, __VA_ARGS__)
#define logWarning(...)     doLog(__FILE__, __LINE__, LOG_LEVEL_WARNING, __VA_ARGS__)
#define logError(...)       doLog(__FILE__, __LINE__, LOG_LEVEL_ERROR, __VA_ARGS__)

#define logDebugV(fmt, args)    doLogV(__FILE__, __LINE__, LOG_LEVEL_DEBUG, fmt, args)
#define logInfoV(fmt, args)     doLogV(__FILE__, __LINE__, LOG_LEVEL_INFO, fmt, args)
#define logNoticeV(fmt, args)   doLogV(__FILE__, __LINE__, LOG_LEVEL_NOTICE, fmt, args)
#define logWarningV(fmt, args)  doLogV(__FILE__, __LINE__, LOG_LEVEL_WARNING, fmt, args)
#define logErrorV(fmt, args)    doLogV(__FILE__, __LINE__, LOG_LEVEL_ERROR, fmt, args)


bool logInit(const std::string& method) noexcept;
void logClose() noexcept;
LogLevel_t logSetLevel(const LogLevel_t newLevel) noexcept;
LogLevel_t logSetLevel(const std::string& newLevel) noexcept;
LogLevel_t logGetLevel() noexcept;

bool doLog(const char * const file, const int line, const LogLevel_t level, const std::string& fmt, ...) noexcept;
bool doLogV(const char * const file, const int line, const LogLevel_t level, const std::string& fmt, va_list args)
        noexcept;

#endif // LOG_H_INC

