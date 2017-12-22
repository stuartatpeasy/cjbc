/*
    button.cc: manages a hardware button or switch

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/peripherals/button.h"
#include "include/framework/log.h"
#include "include/framework/registry.h"


const ButtonId_t Button::invalid_button = GPIOPin::invalid_pin;


// ctor
//
Button::Button(const ButtonId_t button) noexcept
    : pin_(Registry::instance().gpio().pin(button)),
      callback_(nullptr),
      currentState_(BUTTON_RELEASED),
      callbackTriggerState_(BUTTON_ANY_STATE),
      callbackArg_(nullptr)
{
    update();
}


// update() - read and record the button's state; trigger a callback if appropriate.
//
void Button::update() noexcept
{
    const ButtonState_t state = pin_.read() ? BUTTON_PRESSED : BUTTON_RELEASED;

    if(state != currentState_)
    {
        logDebug("Button %d %s", pin_.id(), (state == BUTTON_PRESSED) ? "pressed" : "released");

        // Trigger callback
        if((callback_ != nullptr) && ((callbackTriggerState_ == BUTTON_ANY_STATE) || (callbackTriggerState_ == state)))
            callback_(pin_.id(), state, callbackArg_);

        currentState_ = state;
    }
}


// registerCallback() - register callback function/method <callback>, to be called following any change of button state
// where the new button state matches <state>.  The argument <arg> is passed to <callback>.
//
void Button::registerCallback(const ButtonState_t state, ButtonCallbackFn_t callback, void * const arg) noexcept
{
    callback_ = callback;
    callbackArg_ = arg;
    callbackTriggerState_ = state;
}

