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


// The highest-numbered (according to wiringPi's scheme) GPIO pin
#define GPIO_PIN_MAX            (29)


class GPIOPort
{
public:
                GPIOPort(Error * const err = nullptr) noexcept;
    GPIOPin     pin(const gpio_pin_id_t num) noexcept;

protected:
    bool        preValidate(const int pin, Error * const err = nullptr) noexcept;

    bool        ready_;
};

#endif // GPIOPORT_H_INC

