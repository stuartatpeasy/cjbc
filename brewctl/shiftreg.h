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
#include <cstdint>

class ShiftReg : public Device
{
public:
                            ShiftReg(GPIOPort& gpio, SPIPort& spi);
    virtual                 ~ShiftReg();

    bool                    write(const uint8_t val);
    uint8_t                 read() const { return currentVal_; };

protected:
    void                    strobeRegClk();

    bool                    ready_;

    GPIOPort&               gpio_;
    SPIPort&                spi_;
    uint8_t                 currentVal_;
};

#endif // SHIFTREG_H_INC

