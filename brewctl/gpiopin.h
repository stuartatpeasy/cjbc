#ifndef GPIOPIN_H_INC
#define GPIOPIN_H_INC
/*
    gpiopin.h: GPIO pin driver for Raspberry Pi.  This is a simple abstraction of a single GPIO port pin.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "error.h"
#include <memory>


typedef int gpio_pin_id_t;


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


class GPIOPin
{
friend class GPIOPort;

public:
    static const gpio_pin_id_t  invalid_pin;

protected:
                                GPIOPin(const gpio_pin_id_t pin) noexcept;
public:
    bool                        read() noexcept;
    void                        write(const bool val) noexcept;
    bool                        setMode(const GPIOPinMode_t mode, Error * const err = nullptr) noexcept;
    bool                        setPullupMode(const GPIOPinPullupMode_t mode, Error * const err = nullptr) noexcept;

protected:
    gpio_pin_id_t               pin_;
};

#endif // GPIO_PIN_H_INC

