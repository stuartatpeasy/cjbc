#ifndef GPIOPORT_H_INC
#define GPIOPORT_H_INC
/*
    gpioport.h: GPIO port driver for Raspberry Pi.  This is a simple abstraction around the wiringPi
    library.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "device.h"
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


// The highest-numbered (according to wiringPi's scheme) GPIO pin
#define GPIO_PIN_MAX            (29)


class GPIOPort : public Device
{
public:
                        GPIOPort();
    virtual             ~GPIOPort();

    bool                read(const int pin);
    void                write(const int pin, const bool val);
    void                setMode(const int pin, const GPIOPinMode_t mode);

protected:
    bool                preValidate(const int pin);

    bool                ready_;
};

#endif // GPIOPORT_H_INC

