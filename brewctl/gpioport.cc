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


// pin() - return a GPIOPin object corresponding to GPIO pin <num>.  If <num> is out-of-range, return an object
// corresponding to a non-existent pin; such an object will appear to be a pin stuck at the logic "0" state.
//
GPIOPin GPIOPort::pin(const gpio_pin_id_t num) noexcept
{
    return GPIOPin(((num >= 0) && (num <= GPIO_PIN_MAX)) ? num : GPIOPin::invalid_pin);
}

