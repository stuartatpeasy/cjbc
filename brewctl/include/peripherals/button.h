#ifndef PERIPHERALS_BUTTON_H_INC
#define PERIPHERALS_BUTTON_H_INC
/*
    button.h: manages a hardware button or switch

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/peripherals/gpiopin.h"


typedef gpio_pin_id_t ButtonId_t;


typedef enum ButtonState
{
    BUTTON_RELEASED     = 0,
    BUTTON_PRESSED      = 1,
    BUTTON_ANY_STATE    = 2
} ButtonState_t;

typedef void (*ButtonCallbackFn_t)(const ButtonId_t buttonId, const ButtonState_t state, void *arg);


class Button
{
public:
                            Button(const ButtonId_t button) noexcept;
    virtual                 ~Button() = default;
                            Button(const Button& rhs) = delete;
                            Button(Button&& rhs) = delete;

    Button&                 operator=(const Button& rhs) = delete;
    Button&                 operator=(Button&& rhs) = delete;

    void                    update() noexcept;
    ButtonState_t           state() const noexcept { return currentState_; };
    void                    triggerCallback() noexcept;
    void                    registerCallback(const ButtonState_t state, ButtonCallbackFn_t callback, void * const arg)
                                noexcept;

    static const ButtonId_t invalid_button;

private:
    GPIOPin&                pin_;
    ButtonCallbackFn_t      callback_;
    ButtonState_t           currentState_;
    ButtonState_t           callbackTriggerState_;
    void *                  callbackArg_;
    bool                    stateChanged_;
};

#endif // PERIPHERALS_BUTTON_H_INC

