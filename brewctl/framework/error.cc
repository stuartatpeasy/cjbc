/*
    error.cc: declares the Error class, which acts as a container for runtime errors and their descriptions.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/framework/error.h"
#include "include/framework/log.h"
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>      // ::strerror()
#include <map>
#include <iomanip>
#include <sstream>

using std::map;
using std::ostringstream;
using std::string;


static map<ErrorCode_t, string> errorMessages =
{
    {NO_ERROR,                          "Success"},         // Not really an error code
    {MISSING_ARGVAL,                    "Missing value for argument '%s'"},
    {UNKNOWN_ARG,                       "Unrecognised argument '%s'"},
    {CFG_FILE_OPEN_FAILED,              "Failed to open config file '%s'"},
    {MALLOC_FAILED,                     "Memory allocation failed"},
    {LIBWIRINGPI_INIT_FAILED,           "Failed to initialise libWiringPi"},
    {NO_SUCH_SESSION,                   "Session %d does not exist"},
    {NO_SUCH_PROFILE,                   "Profile %d does not exist"},
    {BAD_PROFILE_TYPE,                  "Profile %d has invalid type '%s'"},
    {SIGHANDLER_INSTALL_FAILED,         "Failed to install %s signal handler: %s (%d)"},
    {SYSCALL_FAILED,                    "System call %s failed"},
    {NO_SUCH_USER,                      "User '%s' does not exist"},
    {INVALID_USER_ID,                   "Invalid user ID %d"},
    {ALREADY_RUNNING,                   "The process is already running"},
    {NOT_RUNNING,                       "The process is not running"},
    {INCOMPLETE_WRITE,                  "Incomplete write to file %s: tried to write %d bytes; actually wrote %d"},
    {CORRUPT_PIDFILE,                   "PID file '%s' appears to be corrupt.  Delete the file, and then manually stop "
                                        "the service"},
    {CONFIG_KEY_MISSING,                "A required key, '%s', is not present in the configuration"},
    {SWITCH_USER_INSUFFICIENT_PRIV,     "Insufficient privilege to switch to user %s"},
    {DB_OPEN_FAILED,                    "Failed to create or open database file '%s': %s (%d)"},
    {DB_TOO_FEW_COLUMNS,                "Query returned too few columns"},
    {DB_SQLITE_ERROR,                   "SQLite error: %s (%d)"},
    {DB_SQLITESTMT_ERROR,               "SQLiteStmt error: %s (%d)"},
    {SPI_MODE_SET_FAILED,               "Failed to set SPI mode"},
    {SPI_DEVICE_OPEN_FAILED,            "Failed to open SPI device"},
    {SPI_PARAM_SET_FAILED,              "Failed to set SPI port parameter"},
    {GPIO_NOT_READY,                    "GPIO port not ready"},
    {GPIO_PIN_MODE_SET_FAILED,          "Failed to set GPIO pin mode"},
    {GPIO_NO_DATA,                      "Missing data buffers for GPIO operation"},
    {GPIO_IOCTL_FAILED,                 "GPIO port ioctl() failed"},
    {GPIO_INVALID_PIN,                  "Invalid pin number for GPIO operation"},
    {GPIO_INVALID_PIN_MODE,             "Invalid mode specified for pin %d"},
    {ADC_NOT_READY,                     "ADC not ready"},
    {ADC_INVALID_CHANNEL,               "Invalid channel number for ADC conversion"},
    {LCD_INVALID_CURSOR_POS,            "Invalid LCD cursor position requested"},
    {SENSOR_INVALID_TYPE,               "Invalid sensor type '%s'"},
    {NO_SUCH_THERMISTOR,                "Thermistor id %d does not exist"},
    {AVAHI_SIMPLE_POLL_CREATE_FAILED,   "Failed to create Avahi simple poll object"},
    {AVAHI_CLIENT_CREATE_FAILED,        "Failed to create Avahi client"},
    {UNKNOWN_ERROR,                     "Unknown error"},
};


static const int msg_buffer_len = 4096; // Length of buffer in which the error message is written

Error::Error() noexcept
    : msg_(""),
      code_(0)
{
}


Error::Error(const Error& rhs) noexcept
{
    init(rhs);
}


Error::Error(Error&& rhs) noexcept
{
    init(rhs);
    rhs.msg_ = string("");
    rhs.code_ = 0;
}


Error& Error::operator=(const Error& rhs) noexcept
{
    return init(rhs);
}


Error& Error::operator=(Error&& rhs) noexcept
{
    init(rhs);
    rhs.msg_ = string("");
    rhs.code_ = 0;

    return *this;
}


// init() - helper function used by copy ctor and assignment operator to initialise this object from another Error
// object.
//
Error& Error::init(const Error& rhs) noexcept
{
    msg_ = rhs.msg_;
    code_ = rhs.code_;

    return *this;
}


// append() - use <format> and <...> to format a string; append it to the object's error string.
//
void Error::append(const string& format, ...) noexcept
{
    va_list args;
    va_start(args, format);

    appendV(format, args);
}


// appendV() - use <format> and <args> to format a string; append it to the object's error string.
//
void Error::appendV(const string& format, va_list args) noexcept
{
    char buffer[msg_buffer_len];

    const int ret = ::vsnprintf(buffer, msg_buffer_len, format.c_str(), args);
    if(ret >= msg_buffer_len)
        buffer[msg_buffer_len - 1] = '\0';      // Output truncated

    msg_ += buffer;
}


// format() - format and store the error message identified by <code>.
//
void Error::format(const ErrorCode_t code, ...) noexcept
{
    va_list args;
    va_start(args, code);

    formatV(code, args);
}


// formatV() - format and store the error message identified by <code>. 
//
void Error::formatV(const ErrorCode_t code, va_list args) noexcept
{
    const auto& msg = errorMessages.find(code);
    string fmtstr;

    if(msg != errorMessages.end())
        fmtstr = msg->second;
    else
    {
        ostringstream unk;
        unk << "Unknown internal error (code 0x" << std::setw(4) << std::hex << (int) code << ")";
        fmtstr = unk.str();
    }

    formatV(code, fmtstr, args);
}


// formatV() - [private] use <args> to fill in the format string <format>, and store it; store <code>.
//
void Error::formatV(const ErrorCode_t code, const string& format, va_list args) noexcept
{
    code_ = code;
    msg_ = "";
    appendV(format, args);
}


// stringFromCode() - return an error string corresponding to the error code in <code>.
//
string Error::stringFromCode(const ErrorCode_t code) noexcept
{
    auto it = errorMessages.find(code);

    if(it == errorMessages.end())
    {
        ostringstream msg;

        msg << "Unexpected error code " << code;
        return msg.str();
    }

    return it->second;
}


// reset() - reset the error object to a "no error" state
//
void Error::reset() noexcept
{
    format(NO_ERROR);
}


// doFormatError() - global error-formatter function.  If <err> is non-null, populate it with an error message according
// to <code> and <...>. If <err> is null, take no action.
//
void doFormatError(const char * const file, const int line, Error * const err, const ErrorCode_t code, ...) noexcept
{
    Error tmp;
    va_list args;
    va_start(args, code);

    Error& e = (err != nullptr) ? *err : tmp;

    e.formatV(code, args);
    doLog(file, line, LOG_LEVEL_ERROR, "%s [%d]", e.message().c_str(), e.code());
}


// formatErrorWithErrno() - like formatError(), but appends a string containing the description of the error represented
// by the current value of errno, and the value of errno itself.
//
void doFormatErrorWithErrno(const char * const file, const int line, Error * const err, const ErrorCode_t code, ...)
    noexcept
{
    Error tmp;
    va_list args;
    va_start(args, code);

    Error& e = (err != nullptr) ? *err : tmp;

    e.formatV(code, args);
    e.append(": %s (%d)", ::strerror(errno), errno);
    doLog(file, line, LOG_LEVEL_ERROR, "%s [%d]", e.message().c_str(), e.code());
}

