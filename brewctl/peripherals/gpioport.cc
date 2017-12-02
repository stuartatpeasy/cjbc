/*
    gpioport.cc: GPIO port driver for Raspberry Pi.  This is a simple abstraction around the wiringPi library.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/peripherals/gpioport.h"
#include "include/log.h"
#include <memory>

extern "C"
{
#include <wiringPi.h>
}


GPIOPort * GPIOPort::instance_ = nullptr;


// The highest-numbered (according to wiringPi's scheme) GPIO pin
const gpio_pin_id_t GPIOPort::GPIO_PIN_MAX = 29;


// ctor - ensure that libWiringPi is initialised
//
GPIOPort::GPIOPort(Error * const err) noexcept
    : invalidPin_(GPIOPin::invalid_pin), ready_(false)
{
    if(::wiringPiSetup() == -1)
    {
        formatError(err, LIBWIRINGPI_INIT_FAILED);
        return;
    }

    for(auto i = 0; i <= GPIO_PIN_MAX; ++i)
        pins_.push_back(new GPIOPin(i));

    ready_ = true;
}


// instance() - implements singleton pattern for the GPIOPort class.  Returns the single instance of the class.
//
GPIOPort& GPIOPort::instance(Error * const err) noexcept
{
    if(instance_ == nullptr)
        instance_ = new GPIOPort(err);

    return *instance_;
}


// pin() - return a GPIOPin object corresponding to GPIO pin <num>.  If <num> is out-of-range, return an object
// corresponding to a non-existent pin; such an object will appear to be a pin stuck at the logic "0" state.
//
GPIOPin& GPIOPort::pin(const gpio_pin_id_t num) noexcept
{
    if((num >= 0) && (num <= GPIO_PIN_MAX))
        return *pins_[num];

    logWarning("Instantiating invalid GPIO pin %d", num);
    return invalidPin_;
}

