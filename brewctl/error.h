#ifndef ERROR_H_INC
#define ERROR_H_INC
/*
    error.h: declares the Error class, which acts as a container for runtime errors and their descriptions.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include <string>


typedef enum
{
    NO_ERROR                    = 0x0000,
    MISSING_ARGVAL              = 0x0001,
    UNKNOWN_ARG                 = 0x0002,
    CFG_FILE_OPEN_FAILED        = 0x0003,
    MALLOC_FAILED               = 0x0004,
    LIBWIRINGPI_INIT_FAILED     = 0x0005,
    NO_SUCH_SESSION             = 0x0006,
    NO_SUCH_PROFILE             = 0x0007,
    BAD_PROFILE_TYPE            = 0x0008,
    DB_OPEN_FAILED              = 0x1100,
    DB_TOO_FEW_COLUMNS          = 0x1101,
    DB_SQLITE_ERROR             = 0x1102,
    DB_SQLITESTMT_ERROR         = 0x1103,
    SPI_MODE_SET_FAILED         = 0x1200,
    SPI_DEVICE_OPEN_FAILED      = 0x1201,
    SPI_PARAM_SET_FAILED        = 0x1202,
    GPIO_NOT_READY              = 0x1300,
    GPIO_PIN_MODE_SET_FAILED    = 0x1301,
    GPIO_NO_DATA                = 0x1302,
    GPIO_IOCTL_FAILED           = 0x1304,
    GPIO_INVALID_PIN            = 0x1305,
    GPIO_INVALID_PIN_MODE       = 0x1306,
    ADC_NOT_READY               = 0x1400,
    ADC_INVALID_CHANNEL         = 0x1401,
    LCD_INVALID_CURSOR_POS      = 0x1500,
    SENSOR_INVALID_TYPE         = 0x1600,
    NO_SUCH_THERMISTOR          = 0x1700,
    UNKNOWN_ERROR               = 0xffff
} ErrorCode_t;


class Error
{
public:
                        Error() noexcept;
                        Error(const Error& rhs) noexcept;
                        Error(Error&& rhs) noexcept;

    Error&              operator=(const Error& rhs) noexcept;
    Error&              operator=(Error&& rhs) noexcept;

    void                format(const ErrorCode_t code, ...) noexcept;
    void                formatV(const ErrorCode_t code, va_list args) noexcept;
    void                formatV(const ErrorCode_t code, const std::string& format, va_list args) noexcept;
    static std::string  stringFromCode(const ErrorCode_t code) noexcept;
    const std::string&  message() const noexcept { return msg_; };
    int                 code() const noexcept { return code_; };
    void                reset() noexcept;

private:
    Error&              init(const Error& rhs) noexcept;

    std::string         msg_;
    int                 code_;
};

void formatError(Error * const err, const ErrorCode_t code, ...) noexcept;

#endif // ERROR_H_INC

