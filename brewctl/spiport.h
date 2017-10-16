#ifndef SPIPORT_H_INC
#define SPIPORT_H_INC
/*
    spiport.cc: spi port abstraction class.  wraps calls to the linux kernel spi driver; also
    uses libwiringpi functions to manage port pins.

    stuart wallace <stuartw@atom.net>, september 2017.

    part of brewctl
*/


#include "config.h"
#include "device.h"
#include "error.h"
#include "gpioport.h"
#include <cstdint>
#include <string>

extern "C"
{
#include <linux/spi/spidev.h>
}

class SPIPort : public Device
{
public:
                            SPIPort(GPIOPort& gpio, Config& config, Error * const err);
    virtual                 ~SPIPort();

    bool                    setMode(const uint8_t mode);
    bool                    setBitsPerWord(const uint8_t bpw);
    bool                    setMaxSpeed(const uint32_t hz);

    bool                    transmitByte(const uint8_t data);
    bool                    receiveByte(uint8_t&  data);
    bool                    transmitAndReceive(const uint8_t *tx_data, uint8_t *rx_data,
                                               const unsigned int len);

    bool                    ready() const { return ready_; };

protected:
    bool                    doIoctl(const unsigned long type, void *val);

    GPIOPort&               gpio_;
    Config&                 config_;

    int                     fd_;
    uint8_t                 mode_;
    uint8_t                 bpw_;
    uint32_t                maxClock_;

    bool                    ready_;

    struct spi_ioc_transfer xfer_;
};

#endif // SPIPORT_H_INC

