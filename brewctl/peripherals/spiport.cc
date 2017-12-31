/*
    spiport.cc: SPI port abstraction class.  Wraps calls to the linux kernel SPI driver; also uses libWiringPi functions
    to manage port pins.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl

    
    SPI clock modes

    +------+----------+----------+-----------------------------------------------------------+
    | Mode | Polarity | Phase    | Description                                               |
    +------+----------+----------+-----------------------------------------------------------+
    |    0 | +ve      | leading  | Changes on -ve edge, samples on +ve edge; clk active-high |
    |    1 | +ve      | trailing | Changes on +ve edge, samples on -ve edge; clk active-high |
    |    2 | -ve      | leading  | Changes on +ve edge, samples on -ve edge; clk active-low  |
    |    3 | -ve      | trailing | Changes on -ve edge, samples on +ve edge; clk active-low  |
    +------+----------+----------+-----------------------------------------------------------+
*/

#include "include/framework/error.h"
#include "include/peripherals/spiport.h"
#include "include/util/validator.h"
#include <cstring>
#include <initializer_list>

extern "C"
{
#include <strings.h>        // bzero()
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
}

using std::string;
namespace Validator = Util::Validator;


static const int
    SPI_DEFAULT_BPW     = 8,                    // Default SPI word length = 8 bits
    SPI_DEFAULT_CLOCK   = 500000,               // Default SPI clock = 500kHz
    SPI_DEFAULT_MODE    = SPI_MODE_0;           // Default SPI mode = mode 0

static const char * const
    SPI_DEFAULT_DEVICE    = "/dev/spidev0.0";   // Default SPI device

//
// Mapping of GPIO pin names to wiringPi pin numbers for SPI pins
//
typedef enum SPIPin
{
    GPIO_MOSI = 12,         // SPI master-out, slave-in
    GPIO_MISO = 13,         // SPI master-in, slave-out
    GPIO_SCLK = 14          // SPI serial clock
} SPIPin_t;


// ctor - note that we can't use Registry members here; instead we must take explicit args for objects which are
// normally read from the registry.  This is because the SPI port is normally init'ed from within the Registry ctor,
// hence we wouldn't be able to obtain a registry instance here.
//
SPIPort::SPIPort(GPIOPort& gpio, Config& config, Error * const err) noexcept
    : fd_(0),
      mode_(0),
      bpw_(0),
      maxClock_(0),
      ready_(false)
{
    for(auto pin : {GPIO_MOSI, GPIO_MISO, GPIO_SCLK})
        if(!gpio.pin(pin).setMode(PIN_ALT0, err))
            return;

    fd_ = ::open(config.get<string>("spi.dev", SPI_DEFAULT_DEVICE, Validator::notEmpty).c_str(), O_RDWR);

    ::bzero(&xfer_, sizeof(xfer_));

    if(fd_ == -1)
    {
        formatError(err, SPI_DEVICE_OPEN_FAILED);
        return;
    }

    // Attempt to set port defaults
    if(!setBitsPerWord(SPI_DEFAULT_BPW, err) ||
       !setMaxSpeed(config.get("spi.max_clock", SPI_DEFAULT_CLOCK, Validator::gt0), err) ||
       !setMode(config.get("spi.mode", SPI_DEFAULT_MODE, Validator::ge0), err))
    {
        formatError(err, SPI_PARAM_SET_FAILED);
        return;
    }

    ready_ = true;
}


// dtor - close the SPI port file descriptor, if it is open.
//
SPIPort::~SPIPort() noexcept
{
    if(fd_ > 0)
        ::close(fd_);
}


// doIoctl() - do an ioctl with a void * arg on our fd.  Return true on success, false otherwise.
//
bool SPIPort::doIoctl(const unsigned long type, void *val, Error * const err) noexcept
{
    if(::ioctl(fd_, type, val) < 0)
    {
        formatError(err, GPIO_IOCTL_FAILED);
        return false;
    }

    return true;
}


// setMode() - set the SPI mode (0..3) for the port.  Return true on success, false otherwise.
//
bool SPIPort::setMode(const uint8_t mode, Error * const err) noexcept
{
    uint8_t mode_local = mode;
    
    if(doIoctl(SPI_IOC_WR_MODE, &mode_local, err))
    {
        mode_ = mode_local;
        return true;
    }

    return false;
}


// setBitsPerWord() - set the number of bits in each word transmitted or received on the SPI port.  Return true on
// success, false otherwise.
//
bool SPIPort::setBitsPerWord(const uint8_t bpw, Error * const err) noexcept
{
    uint8_t bpw_local = bpw;

    if(doIoctl(SPI_IOC_WR_BITS_PER_WORD, &bpw_local, err))
    {
        xfer_.bits_per_word = bpw_local;
        bpw_ = bpw_local;
        return true;
    }

    return false;
}


// setMaxSpeed() - set the highest allowable clock speed for the port to <hz> Hz.  Note that the actual clock speed used
// by the port may be lower.  Return true on success, false otherwise.
//
bool SPIPort::setMaxSpeed(const uint32_t hz, Error * const err) noexcept
{
    uint32_t hz_local = hz;

    if(doIoctl(SPI_IOC_WR_MAX_SPEED_HZ, &hz_local, err))
    {
        xfer_.speed_hz = hz;
        maxClock_ = hz_local;
        return true;
    }

    return false;
}


// transmitAndReceive() - do a simultaneous transmit/receive of len bytes of data.  Return true on success, false
// otherwise.
//
bool SPIPort::transmitAndReceive(const uint8_t *tx_data, uint8_t *rx_data, const unsigned int len, Error * const err)
    noexcept
{
    uint8_t *rx_data_local;
    const uint8_t *tx_data_local;

    if(!len)
        return true;        // Zero-length transfers always succeed

    if((tx_data == NULL) && (rx_data == NULL))
    {
        formatError(err, GPIO_NO_DATA);
        return false;
    }

    if(!ready())
    {
        formatError(err, GPIO_NOT_READY);
        return false;
    }

    if(rx_data == NULL)
    {
        rx_data_local = new uint8_t[len];
        if(!rx_data_local)
        {
            formatError(err, MALLOC_FAILED);
            return false;
        }
    }
    else
        rx_data_local = rx_data;

    if(tx_data == NULL)
    {
        tx_data_local = new uint8_t[len];
        if(!tx_data_local)
        {
            if(!rx_data)
                delete[] rx_data_local;
            delete[] tx_data_local;

            formatError(err, MALLOC_FAILED);
            return false;
        }
    }
    else
        tx_data_local = tx_data;

    xfer_.rx_buf = (unsigned long) rx_data_local;
    xfer_.tx_buf = (unsigned long) tx_data_local;
    xfer_.len = len;

    const bool ret = doIoctl(SPI_IOC_MESSAGE(1), &xfer_, err);

    if(rx_data == NULL)
        delete[] rx_data_local;

    if(tx_data == NULL)
        delete[] tx_data_local;

    return ret;
}


// transmitByte() - transmit the byte in <data> to the SPI port.  Return true on success, false otherwise.
//
bool SPIPort::transmitByte(const uint8_t data, Error * const err) noexcept
{
    return transmitAndReceive(&data, NULL, 1, err);
}


// receiveByte() - receive a byte from the SPI port into <data>.  Return true on success, false otherwise.
//
bool SPIPort::receiveByte(uint8_t& data, Error * const err) noexcept
{
    return transmitAndReceive(NULL, &data, 1, err);
}

