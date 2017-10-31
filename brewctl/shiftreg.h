#ifndef SHIFTREG_H_INC
#define SHIFTREG_H_INC
/*
    74xx595 shift-register SPI driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "error.h"
#include <cstddef>
#include <cstdint>


class ShiftReg
{
public:
                            ShiftReg(Error * const err = nullptr);

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
    bool                    strobeRegClk(Error * const err = nullptr);

    bool                    ready_;
    uint16_t                currentVal_;
};

#endif // SHIFTREG_H_INC

