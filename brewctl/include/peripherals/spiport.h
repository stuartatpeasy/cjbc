#ifndef PERIPHERALS_SPIPORT_H_INC
#define PERIPHERALS_SPIPORT_H_INC
/*
    spiport.cc: SPI port abstraction class.  Wraps calls to the linux kernel SPI driver; also uses libWiringPi functions
    to manage port pins.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/


#include "include/config.h"
#include "include/error.h"
#include "include/peripherals/gpioport.h"
#include <cstddef>
#include <cstdint>
#include <string>

extern "C"
{
#include <linux/spi/spidev.h>
}


class SPIPort
{
public:
                            SPIPort(GPIOPort& gpio, Config& config, Error * const err = nullptr) noexcept;
    virtual                 ~SPIPort() noexcept;

    bool                    setMode(const uint8_t mode, Error * const err = nullptr) noexcept;
    bool                    setBitsPerWord(const uint8_t bpw, Error * const err = nullptr) noexcept;
    bool                    setMaxSpeed(const uint32_t hz, Error * const err = nullptr) noexcept;

    bool                    transmitByte(const uint8_t data, Error * const err = nullptr) noexcept;
    bool                    receiveByte(uint8_t&  data, Error * const err = nullptr) noexcept;
    bool                    transmitAndReceive(const uint8_t *tx_data, uint8_t *rx_data, const unsigned int len,
                                               Error * const err = nullptr) noexcept;

    bool                    ready() const noexcept { return ready_; };

protected:
    bool                    doIoctl(const unsigned long type, void *val, Error * const err = nullptr) noexcept;

    int                     fd_;
    uint8_t                 mode_;
    uint8_t                 bpw_;
    uint32_t                maxClock_;

    bool                    ready_;

    struct spi_ioc_transfer xfer_;
};

#endif // PERIPHERALS_SPIPORT_H_INC

