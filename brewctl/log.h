#ifndef LOG_H_INC
#define LOG_H_INC
/*
    log.h: global logging functions
*/


typedef enum
{
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_NOTICE,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
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

bool logInit(const char * method);
void logClose();

bool doLog(const char * const file, const int line, const LogLevel_t level, const char * const fmt,
           ...);

#endif // LOG_H_INC

