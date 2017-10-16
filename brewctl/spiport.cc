/*
    spiport.cc: spi port abstraction class.  wraps calls to the linux kernel spi driver; also
    uses libwiringpi functions to manage port pins.

    stuart wallace <stuartw@atom.net>, september 2017.

    part of brewctl

    
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

#include "spiport.h"
#include "error.h"
#include <cerrno>
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

#define SPI_DEFAULT_BPW     (8)             // Default SPI word length = 8 bits
#define SPI_DEFAULT_CLOCK   (500000)        // Default SPI clock = 500kHz
#define SPI_DEFAULT_MODE    (SPI_MODE_0)    // Default SPI mode = mode 0

//
// Mapping of GPIO pin names to wiringPi pin numbers for SPI pins
//
typedef enum SPIPin
{
    GPIO_MOSI = 12,         // SPI master-out, slave-in
    GPIO_MISO = 13,         // SPI master-in, slave-out
    GPIO_SCLK = 14          // SPI serial clock
} SPIPin_t;


SPIPort::SPIPort(GPIOPort& gpio, Config& config, Error * const err)
    : Device(), gpio_(gpio), config_(config), fd_(0), mode_(0), bpw_(0), maxClock_(0), ready_(false)
{
    for(auto pin : {GPIO_MOSI, GPIO_MISO, GPIO_SCLK})
        if(!gpio_.setMode(pin, PIN_ALT0))
        {
            err->format(SPI_MODE_SET_FAILED);
            return;
        }

    fd_ = ::open(config_("spi.dev").c_str(), O_RDWR);

    ::bzero(&xfer_, sizeof(xfer_));

    if(fd_ == -1)
    {
        err->format(SPI_DEVICE_OPEN_FAILED);
        return;
    }

    // Attempt to set port defaults
    if(!setBitsPerWord(SPI_DEFAULT_BPW) ||
       !setMaxSpeed(config_.get("spi.max_clock", SPI_DEFAULT_CLOCK)) ||
       !setMode(config_.get("spi.mode", SPI_DEFAULT_MODE)))
    {
        err->format(SPI_PARAM_SET_FAILED);
        return;
    }

    ready_ = true;
}


SPIPort::~SPIPort()
{
    if(fd_ > 0)
        ::close(fd_);
}


// doIoctl() - do an ioctl with a void * arg on our fd without affecting the value of ::errno.
// Return 0 on success, or errno on failure.
//
bool SPIPort::doIoctl(const unsigned long type, void *val)
{
    const auto old_errno = errno;

    errno_ = 0;
    if(::ioctl(fd_, type, val) < 0)
    {
        errno_ = errno;
        errno = old_errno;
    }

    return !errno_;
}


bool SPIPort::setMode(const uint8_t mode)
{
    uint8_t mode_local = mode;
    
    if(doIoctl(SPI_IOC_WR_MODE, &mode_local))
    {
        mode_ = mode_local;
        return true;
    }

    return false;
}


bool SPIPort::setBitsPerWord(const uint8_t bpw)
{
    uint8_t bpw_local = bpw;

    if(doIoctl(SPI_IOC_WR_BITS_PER_WORD, &bpw_local))
    {
        xfer_.bits_per_word = bpw_local;
        bpw_ = bpw_local;
        return true;
    }

    return false;
}


bool SPIPort::setMaxSpeed(const uint32_t hz)
{
    uint32_t hz_local = hz;

    if(doIoctl(SPI_IOC_WR_MAX_SPEED_HZ, &hz_local))
    {
        xfer_.speed_hz = hz;
        maxClock_ = hz_local;
        return true;
    }

    return false;
}


// transmitAndReceive() - do a simultaneous transmit/receive of len bytes of data.
//
bool SPIPort::transmitAndReceive(const uint8_t *tx_data, uint8_t *rx_data, const unsigned int len)
{
    uint8_t *rx_data_local;
    const uint8_t *tx_data_local;

    if(!len)
        return true;        // Zero-length transfers always succeed

    if((tx_data == NULL) && (rx_data == NULL))
    {
        errno_ = EINVAL;
        return false;
    }

    if(!ready())
    {
        errno_ = EAGAIN;
        return false;
    }

    if(rx_data == NULL)
    {
        rx_data_local = new uint8_t[len];
        if(!rx_data_local)
        {
            errno_ = ENOMEM;
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

            errno_ = ENOMEM;
            return false;
        }
    }
    else
        tx_data_local = tx_data;

    xfer_.rx_buf = (unsigned long) rx_data_local;
    xfer_.tx_buf = (unsigned long) tx_data_local;
    xfer_.len = len;

    const bool ret = doIoctl(SPI_IOC_MESSAGE(1), &xfer_);

    if(ret)
        errno_ = 0;

    if(rx_data == NULL)
        delete[] rx_data_local;

    if(tx_data == NULL)
        delete[] tx_data_local;

    return ret;
}


bool SPIPort::transmitByte(const uint8_t data)
{
    return transmitAndReceive(&data, NULL, 1);
}


bool SPIPort::receiveByte(uint8_t& data)
{
    return transmitAndReceive(NULL, &data, 1);
}

