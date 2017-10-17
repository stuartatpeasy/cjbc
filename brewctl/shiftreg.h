#ifndef SHIFTREG_H_INC
#define SHIFTREG_H_INC
/*
    74xx595 shift-register SPI driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "device.h"
#include "gpioport.h"
#include "spiport.h"
#include <cstddef>
#include <cstdint>


class ShiftReg : public Device
{
public:
                            ShiftReg(GPIOPort& gpio, SPIPort& spi, Error * const err = nullptr);
    virtual                 ~ShiftReg();

    bool                    write(uint16_t val, Error * const err = nullptr);
    uint16_t                read() const { return currentVal_; };

    uint16_t                operator|=(const uint16_t rhs);
    uint16_t                operator&=(const uint16_t rhs);
    uint16_t                operator^=(const uint16_t rhs);
                            operator uint16_t() const { return currentVal_; };

    bool                    set(const unsigned int bit, Error * const err = nullptr);
    bool                    clear(const unsigned int bit, Error * const err = nullptr);
    bool                    toggle(const unsigned int bit, Error * const err = nullptr);
    bool                    isSet(const unsigned int bit, Error * const err = nullptr);

protected:
    void                    strobeRegClk();

    bool                    ready_;

    GPIOPort&               gpio_;
    SPIPort&                spi_;
    uint16_t                currentVal_;
};

#endif // SHIFTREG_H_INC

