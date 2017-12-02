/*
    gpiopin.cc: GPIO pin driver for Raspberry Pi.  This is a simple abstraction of a single GPIO port pin.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/gpiopin.h"

extern "C"
{
#include <wiringPi.h>
}

using std::lock_guard;
using std::mutex;


const gpio_pin_id_t GPIOPin::invalid_pin = -1;


// ctor - store pin id.
//
GPIOPin::GPIOPin(const gpio_pin_id_t pin) noexcept
    : pin_(pin)
{
}


// read() - read the current boolean value on the pin.  The underlying ::digitalRead() fn returns bool, so error-
// detection is not possible in this method.
//
bool GPIOPin::read() noexcept
{
    if(pin_ == invalid_pin)
        return false;

    lock_guard<mutex> lock(lock_);

    return ::digitalRead(pin_) == HIGH;
}


// write() - write the boolean value <val> to the pin.  The underlying ::digitalWrite() fn returns void, so error-
// detection is not possible in this method.
//
void GPIOPin::write(const bool val) noexcept
{
    if(pin_ != invalid_pin)
    {
        lock_guard<mutex> lock(lock_);
        ::digitalWrite(pin_, val ? HIGH : LOW);
    }
}


// setMode() - set the pin's mode to <mode>, e.g. input / output / ...  Return true on success, false otherwise.
//
bool GPIOPin::setMode(const GPIOPinMode_t mode, Error * const err) noexcept
{
    if(pin_ != invalid_pin)
    {
        lock_guard<mutex> lock(lock_);

        switch(mode)
        {
            case PIN_INPUT:             ::pinMode(pin_, INPUT);              break;
            case PIN_OUTPUT:            ::pinMode(pin_, OUTPUT);             break;
            case PIN_PWM_OUTPUT:        ::pinMode(pin_, PWM_OUTPUT);         break;
            case PIN_PWM_TONE_OUTPUT:   ::pinMode(pin_, PWM_TONE_OUTPUT);    break;
            case PIN_CLOCK:             ::pinMode(pin_, GPIO_CLOCK);         break;
            case PIN_ALT0:              ::pinModeAlt(pin_, 0b100);           break;
            case PIN_ALT1:              ::pinModeAlt(pin_, 0b101);           break;
            case PIN_ALT2:              ::pinModeAlt(pin_, 0b110);           break;
            case PIN_ALT3:              ::pinModeAlt(pin_, 0b111);           break;
            case PIN_ALT4:              ::pinModeAlt(pin_, 0b011);           break;
            case PIN_ALT5:              ::pinModeAlt(pin_, 0b010);           break;

            default:
                formatError(err, GPIO_INVALID_PIN_MODE, pin_);
                return false;
        }
    }

    return true;
}


// setPullupMode() - activate/deactivate pullup/pulldown on the pin.  Return true on success, false otherwise.
//
bool GPIOPin::setPullupMode(const GPIOPinPullupMode_t mode, Error * const err) noexcept
{
    if(pin_ != invalid_pin)
    {
        lock_guard<mutex> lock(lock_);

        switch(mode)
        {
            case PPM_UP:                ::pullUpDnControl(pin_, PUD_UP);     break;
            case PPM_DOWN:              ::pullUpDnControl(pin_, PUD_DOWN);   break;
            case PPM_NONE:              ::pullUpDnControl(pin_, PUD_OFF);    break;

            default:
                formatError(err, GPIO_INVALID_PIN_MODE, pin_);
                return false;
        }
    }

    return true;
}

