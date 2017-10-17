/*
    error.h: declares the Error class, which acts as a container for runtime errors and their
    descriptions.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "error.h"
#include <cstdarg>
#include <cstdio>
#include <map>
#include <sstream>

using std::map;
using std::ostringstream;
using std::string;


static map<ErrorCode_t, string> errStrings =
{
    {MISSING_ARGVAL,            "Missing value for argument '%s'"},
    {UNKNOWN_ARG,               "Unrecognised argument '%s'"},
    {CFG_FILE_OPEN_FAILED,      "Failed to open config file '%s'"},
    {MALLOC_FAILED,             "Memory allocation failed"},
    {LIBWIRINGPI_INIT_FAILED,   "Failed to initialise libWiringPi"},
    {DB_OPEN_FAILED,            "Failed to create or open database file '%s': %s (%d)"},
    {DB_TOO_FEW_COLUMNS,        "Query returned too few columns"},
    {DB_SQLITE_ERROR,           "SQLite error: %s (%d)"},
    {DB_SQLITESTMT_ERROR,       "SQLiteStmt error: %s (%d)"},
    {SPI_MODE_SET_FAILED,       "Failed to set SPI mode"},
    {SPI_DEVICE_OPEN_FAILED,    "Failed to open SPI device"},
    {SPI_PARAM_SET_FAILED,      "Failed to set SPI port parameter"},
    {GPIO_NOT_READY,            "GPIO port not ready"},
    {GPIO_PIN_MODE_SET_FAILED,  "Failed to set GPIO pin mode"},
    {GPIO_NO_DATA,              "Missing data buffers for GPIO operation"},
    {GPIO_IOCTL_FAILED,         "GPIO port ioctl() failed"},
    {GPIO_INVALID_PIN,          "Invalid pin number for GPIO operation"},
    {GPIO_INVALID_PIN_MODE,     "Invalid mode specified for pin %d"},
    {ADC_NOT_READY,             "ADC not ready"},
    {ADC_INVALID_CHANNEL,       "Invalid channel number for ADC conversion"},
    {LCD_INVALID_CURSOR_POS,    "Invalid LCD cursor position requested"},
    {UNKNOWN_ERROR,             "Unknown error"},
};



static map<ErrorCode_t, const string> errorMessages =
{
    {DB_TOO_FEW_COLUMNS,        "Database error: too few columns returned by query"}
};



static const int msg_buffer_len = 4096; // Length of buffer in which the error message is written

Error::Error()
    : msg_(""), code_(0)
{

}


Error::Error(const Error& rhs)
{
    init(rhs);
}


Error::Error(Error&& rhs) noexcept
{
    init(rhs);
    rhs.msg_ = string("");
    rhs.code_ = 0;
}


Error::~Error()
{
}


Error& Error::operator=(const Error& rhs)
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


// init() - helper function used by copy ctor and assignment operator to initialise this object
// from another Error object.
//
Error& Error::init(const Error& rhs)
{
    msg_ = rhs.msg_;
    code_ = rhs.code_;

    return *this;
}


// format() - format and store the error message identified by <code>.
//
void Error::format(const ErrorCode_t code, ...)
{
    va_list args;
    va_start(args, code);

    formatV(code, args);
}


// formatV() - format and store the error message identified by <code>. 
//
void Error::formatV(const ErrorCode_t code, va_list args)
{
    const auto msg = errorMessages.find(code);
    const string fmtstr = (msg == errorMessages.end()) ? "Unknown internal error" : msg->second;
    formatV(code, fmtstr.c_str(), args);
}


// formatV() - [private] use <args> to fill in the format string <format>, and store it; store
// <code>.
//
void Error::formatV(const ErrorCode_t code, const string& format, va_list args)
{
    char buffer[msg_buffer_len];

    const int ret = ::vsnprintf(buffer, msg_buffer_len, format.c_str(), args);
    if(ret >= msg_buffer_len)
        buffer[msg_buffer_len - 1] = '\0';      // Output truncated

    code_ = code;
    msg_ = buffer;
}


// stringFromCode() - return an error string corresponding to the error code in <code>.
//
string Error::stringFromCode(const ErrorCode_t code)
{
    auto it = errStrings.find(code);

    if(it == errStrings.end())
    {
        ostringstream msg;

        msg << "Unexpected error code " << code;
        return msg.str();
    }

    return it->second;
}


void formatError(Error * const err, const ErrorCode_t code, ...)
{
    if(err != nullptr)
    {
        va_list args;
        va_start(args, code);

        err->formatV(code, args);
    }
}

