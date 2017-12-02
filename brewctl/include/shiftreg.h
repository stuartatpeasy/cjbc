#ifndef SHIFTREG_H_INC
#define SHIFTREG_H_INC
/*
    74xx595 shift-register SPI driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "error.h"
#include "gpioport.h"
#include <cstddef>
#include <cstdint>
#include <mutex>


class ShiftReg
{
public:
                            ShiftReg(GPIOPort& gpio, Error * const err = nullptr) noexcept;

    bool                    init(Error * const err = nullptr) noexcept;
    bool                    write(uint16_t val, Error * const err = nullptr) noexcept;
    uint16_t                read() const noexcept { return currentVal_; };

    uint16_t                operator|=(const uint16_t rhs) noexcept;
    uint16_t                operator&=(const uint16_t rhs) noexcept;
    uint16_t                operator^=(const uint16_t rhs) noexcept;
                            operator uint16_t() const noexcept { return currentVal_; };

    bool                    set(const unsigned int bit, Error * const err = nullptr) noexcept;
    bool                    clear(const unsigned int bit, Error * const err = nullptr) noexcept;
    bool                    toggle(const unsigned int bit, Error * const err = nullptr) noexcept;
    bool                    isSet(const unsigned int bit, Error * const err = nullptr) noexcept;

protected:
    void                    strobeRegClk() noexcept;

    bool                    ready_;
    uint16_t                currentVal_;
    std::mutex              innerLock_;
    std::mutex              lock_;
};

#endif // SHIFTREG_H_INC

