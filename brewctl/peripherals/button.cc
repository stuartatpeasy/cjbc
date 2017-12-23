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
      callbackArg_(nullptr),
      stateChanged_(false)
{
    update();
}


// update() - read and record the button's state; set stateChanged_ to true if a state change has occurred.
//
void Button::update() noexcept
{
    const ButtonState_t state = pin_.read() ? BUTTON_PRESSED : BUTTON_RELEASED;

    if(state != currentState_)
    {
        currentState_ = state;
        stateChanged_ = true;
    }
}


// triggerCallback() - trigger a callback if a state change has occurred, and a callback with appropriate conditions has
// been registered.
//
void Button::triggerCallback() noexcept
{
    if((callback_ != nullptr) && (stateChanged_) &&
       ((callbackTriggerState_ == BUTTON_ANY_STATE) || (callbackTriggerState_ == currentState_)))
    {
        stateChanged_ = false;
        callback_(pin_.id(), currentState_, callbackArg_);
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

