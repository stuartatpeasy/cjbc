#ifndef GPIOPORT_H_INC
#define GPIOPORT_H_INC
/*
    gpioport.h: GPIO port driver for Raspberry Pi.  This is a simple abstraction around the wiringPi library.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "error.h"
#include <cstddef>
#include <cstdint>


//
// "Modes" for GPIO pins - straight input/outputs, PWM outputs, clocks, or alternate functions
//
typedef enum GPIOPinMode
{
    PIN_INPUT,
    PIN_OUTPUT,
    PIN_PWM_OUTPUT,
    PIN_PWM_TONE_OUTPUT,
    PIN_CLOCK,
    PIN_ALT0,
    PIN_ALT1,
    PIN_ALT2,
    PIN_ALT3,
    PIN_ALT4,
    PIN_ALT5
} GPIOPinMode_t;


//
// Pull-up/pull-down modes for GPIO pins
//
typedef enum GPIOPinPullupMode
{
    PPM_UP,
    PPM_DOWN,
    PPM_NONE
} GPIOPinPullupMode_t;


// The highest-numbered (according to wiringPi's scheme) GPIO pin
#define GPIO_PIN_MAX            (29)


class GPIOPort
{
public:
                GPIOPort(Error * const err = nullptr) noexcept;

    bool        read(const int pin, Error * const err = nullptr) noexcept;
    bool        write(const int pin, const bool val, Error * const err = nullptr) noexcept;
    bool        setMode(const int pin, const GPIOPinMode_t mode, Error * const err = nullptr) noexcept;
    bool        setPullupMode(const int pin, const GPIOPinPullupMode_t mode, Error * const err = nullptr) noexcept;

protected:
    bool        preValidate(const int pin, Error * const err = nullptr) noexcept;

    bool        ready_;
};

#endif // GPIOPORT_H_INC

