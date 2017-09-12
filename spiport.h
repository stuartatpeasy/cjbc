#ifndef SPIPORT_H_INC
#define SPIPORT_H_INC
/*
    spiport.cc: spi port abstraction class.  wraps calls to the linux kernel spi driver; also
    uses libwiringpi functions to manage port pins.

    stuart wallace <stuartw@atom.net>, september 2017.

    part of brewctl
*/


#include "device.h"
#include "gpioport.h"
#include <cstdint>

extern "C"
{
#include <linux/spi/spidev.h>
};

class SPIPort : public Device
{
public:
                            SPIPort(GPIOPort& gpio, const char * const device);
    virtual                 ~SPIPort();

    bool                    setMode(const uint8_t mode);
    bool                    setBitsPerWord(const uint8_t bpw);
    bool                    setMaxSpeed(const uint32_t hz);

    bool                    transmitByte(const uint8_t data);
    bool                    receiveByte(uint8_t&  data);
    bool                    transmitAndReceive(const uint8_t *tx_data, uint8_t *rx_data,
                                               const unsigned int len);

    bool                    ready() const { return modeSet_ && bpwSet_ && hzSet_; };

protected:
    bool                    doIoctl(const unsigned long type, void *val);

    GPIOPort&               gpio_;

    int                     fd_;

    bool                    modeSet_;
    bool                    bpwSet_;
    bool                    hzSet_;

    struct spi_ioc_transfer xfer_;
};

#endif // SPIPORT_H_INC

