#ifndef GPIOPORT_H_INC
#define GPIOPORT_H_INC
/*
    gpioport.h: GPIO port driver for Raspberry Pi.  This is a simple abstraction around the wiringPi library.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "error.h"
#include "gpiopin.h"
#include <cstddef>
#include <cstdint>
#include <vector>


class GPIOPort
{
public:
    static GPIOPort&            instance(Error * const err = nullptr) noexcept;
    GPIOPin&                    pin(const gpio_pin_id_t num) noexcept;

private:
                                GPIOPort(Error * const err = nullptr) noexcept;
    static GPIOPort *           instance_;

    static const gpio_pin_id_t  GPIO_PIN_MAX;
    GPIOPin                     invalidPin_;
    bool                        ready_;
    std::vector<GPIOPin *>      pins_;
};

#endif // GPIOPORT_H_INC

