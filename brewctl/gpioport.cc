/*
    gpioport.cc: GPIO port driver for Raspberry Pi.  This is a simple abstraction around the wiringPi library.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "gpioport.h"

extern "C"
{
#include <wiringPi.h>
}


GPIOPort::GPIOPort(Error * const err) noexcept
    : ready_(false)
{
    if(::wiringPiSetup() == -1)
    {
        formatError(err, LIBWIRINGPI_INIT_FAILED);
        return;
    }

    ready_ = true;
}


// preValidate() - check ready state and supplied pin number.  Return true if the device is ready and the pin number is
// valid; false otherwise.
//
bool GPIOPort::preValidate(const int pin, Error * const err) noexcept
{
    if(!ready_)
    {
        formatError(err, GPIO_NOT_READY);
        return false;
    }

    if((pin < 0) || (pin > GPIO_PIN_MAX))
    {
        formatError(err, GPIO_INVALID_PIN);
        return false;
    }

    return true;
}


// read() - read the logic level of pin <pin> and return it.  Return false if the pin number is out of range, or the
// port is not ready.
// FIXME: this is not ideal - the return value is overloaded.
//
bool GPIOPort::read(const int pin, Error * const err) noexcept
{
    if(!preValidate(pin, err))
        return false;

    return ::digitalRead(pin) == HIGH;
}


// write() - write the logic level <val> to pin <pin>.  Return true on success, false otherwise.
//
bool GPIOPort::write(const int pin, const bool val, Error * const err) noexcept
{
    if(!preValidate(pin, err))
        return false;

    ::digitalWrite(pin, val ? HIGH : LOW);
    return true;
}


// setMode() - set pin <pin>'s mode to <mode>, e.g. input / output / ...  Return true on success, false otherwise.
//
bool GPIOPort::setMode(const int pin, const GPIOPinMode_t mode, Error * const err) noexcept
{
    if(!preValidate(pin, err))
        return false;

    switch(mode)
    {
        case PIN_INPUT:             ::pinMode(pin, INPUT);              break;
        case PIN_OUTPUT:            ::pinMode(pin, OUTPUT);             break;
        case PIN_PWM_OUTPUT:        ::pinMode(pin, PWM_OUTPUT);         break;
        case PIN_PWM_TONE_OUTPUT:   ::pinMode(pin, PWM_TONE_OUTPUT);    break;
        case PIN_CLOCK:             ::pinMode(pin, GPIO_CLOCK);         break;
        case PIN_ALT0:              ::pinModeAlt(pin, 0b100);           break;
        case PIN_ALT1:              ::pinModeAlt(pin, 0b101);           break;
        case PIN_ALT2:              ::pinModeAlt(pin, 0b110);           break;
        case PIN_ALT3:              ::pinModeAlt(pin, 0b111);           break;
        case PIN_ALT4:              ::pinModeAlt(pin, 0b011);           break;
        case PIN_ALT5:              ::pinModeAlt(pin, 0b010);           break;

        default:
            formatError(err, GPIO_INVALID_PIN_MODE, pin);
            return false;
    }

    return true;
}


// setPullupMode() - activate/deactivate pullup/pulldown on pin <pin>.  Return true on success, false otherwise.
//
bool GPIOPort::setPullupMode(const int pin, const GPIOPinPullupMode_t mode, Error * const err) noexcept
{
    if(!preValidate(pin, err))
        return false;

    switch(mode)
    {
        case PPM_UP:                ::pullUpDnControl(pin, PUD_UP);     break;
        case PPM_DOWN:              ::pullUpDnControl(pin, PUD_DOWN);   break;
        case PPM_NONE:              ::pullUpDnControl(pin, PUD_OFF);    break;

        default:
            formatError(err, GPIO_INVALID_PIN_MODE, pin);
            return false;
    }

    return true;
}

