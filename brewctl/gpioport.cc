/*
    gpioport.cc: GPIO port driver for Raspberry Pi.  This is a simple abstraction around the
    wiringPi library.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "gpioport.h"
#include <cerrno>

extern "C"
{
#include <wiringPi.h>
};


GPIOPort::GPIOPort()
    : Device(), ready_(false)
{
    if(::wiringPiSetup() == -1)
    {
        errno_ = errno;
        return;
    }

    ready_ = true;
}


GPIOPort::~GPIOPort()
{
}


// preValidate() - check ready state and supplied pin number; set errno accordingly.  Return true if
// the device is ready and the pin number is valid; false otherwise.
//
bool GPIOPort::preValidate(const int pin)
{
    if(!ready_)
    {
        errno_ = EAGAIN;
        return false;
    }

    if((pin < 0) || (pin > GPIO_PIN_MAX))
    {
        errno_ = EINVAL;
        return false;
    }

    return true;
}


bool GPIOPort::read(const int pin)
{
    if(!preValidate(pin))
        return false;

    errno_ = 0;
    return ::digitalRead(pin) == HIGH;
}


void GPIOPort::write(const int pin, const bool val)
{
    if(!preValidate(pin))
        return;

    ::digitalWrite(pin, val ? HIGH : LOW);
    errno_ = 0;
}


bool GPIOPort::setMode(const int pin, const GPIOPinMode_t mode)
{
    if(!preValidate(pin))
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

        default:                    errno_ = EINVAL;                    return false;
    }

    errno_ = 0;
    return true;
}


bool GPIOPort::setPullupMode(const int pin, const GPIOPinPullupMode_t mode)
{
    if(!preValidate(pin))
        return false;

    switch(mode)
    {
        case PPM_UP:                ::pullUpDnControl(pin, PUD_UP);     break;
        case PPM_DOWN:              ::pullUpDnControl(pin, PUD_DOWN);   break;
        case PPM_NONE:              ::pullUpDnControl(pin, PUD_OFF);    break;

        default:                    errno_ = EINVAL;                    return false;
    }

    errno_ = 0;
    return true;
}


