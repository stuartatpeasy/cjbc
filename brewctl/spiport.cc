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
#include <initializer_list>
#include <cerrno>
#include <cstring>

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


SPIPort::SPIPort(GPIOPort& gpio, const string& device)
    : Device(), gpio_(gpio), fd_(0), modeSet_(false), bpwSet_(false), hzSet_(false)
{
    const auto old_errno = errno;

    for(auto pin : {GPIO_MOSI, GPIO_MISO, GPIO_SCLK})
        if(!gpio_.setMode(pin, PIN_ALT0))
            return;

    errno = 0;
    fd_ = ::open(device.c_str(), O_RDWR);

    ::bzero(&xfer_, sizeof(xfer_));

    if(fd_ == -1)
    {
        errno_ = errno;
        errno = old_errno;
        return;
    }

    // Attempt to set port defaults
    bpwSet_ = setBitsPerWord(SPI_DEFAULT_BPW);
    hzSet_ = setMaxSpeed(SPI_DEFAULT_CLOCK);
    modeSet_ = setMode(SPI_DEFAULT_MODE);
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
    
    const auto ret = doIoctl(SPI_IOC_WR_MODE, &mode_local);

    if(ret)
        modeSet_ = true;

    return ret;
}


bool SPIPort::setBitsPerWord(const uint8_t bpw)
{
    uint8_t bpw_local = bpw;

    const bool ret = doIoctl(SPI_IOC_WR_BITS_PER_WORD, &bpw_local);
    if(ret)
    {
        xfer_.bits_per_word = bpw;
        bpwSet_ = true;
    }

    return ret;
}


bool SPIPort::setMaxSpeed(const uint32_t hz)
{
    uint32_t hz_local = hz;

    const bool ret = doIoctl(SPI_IOC_WR_MAX_SPEED_HZ, &hz_local);
    if(ret)
    {
        xfer_.speed_hz = hz;
        hzSet_ = true;
    }

    return ret;
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

