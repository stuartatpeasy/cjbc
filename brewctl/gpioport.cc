/*
    gpioport.cc: GPIO port driver for Raspberry Pi.  This is a simple abstraction around the wiringPi library.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "gpioport.h"
#include "log.h"

extern "C"
{
#include <wiringPi.h>
}


// The highest-numbered (according to wiringPi's scheme) GPIO pin
const gpio_pin_id_t GPIOPort::GPIO_PIN_MAX = 29;


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


// pin() - return a GPIOPin object corresponding to GPIO pin <num>.  If <num> is out-of-range, return an object
// corresponding to a non-existent pin; such an object will appear to be a pin stuck at the logic "0" state.
//
GPIOPin GPIOPort::pin(const gpio_pin_id_t num) noexcept
{
    if((num >= 0) && (num <= GPIO_PIN_MAX))
        return GPIOPin(num);

    logWarning("Instantiating invalid GPIO pin %d", num);
    return GPIOPin(GPIOPin::invalid_pin);
}

