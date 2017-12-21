#ifndef PERIPHERALS_BUTTONMANAGER_H_INC
#define PERIPHERALS_BUTTONMANAGER_H_INC
/*
    buttonmanager.h: operates a thread which monitors button inputs.

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/peripherals/gpioport.h"
#include "include/framework/thread.h"
#include <map>


typedef enum ButtonId
{
    SWITCH_BOTTOM       = 2,
    SWITCH_TOP          = 3,
    ROT_CW              = 4,
    ROT_CCW             = 5,
    ROT_BUTTON          = 6
} ButtonId_t;


class ButtonManager : public Thread
{
public:
                                    ButtonManager() noexcept;
                                    ButtonManager(const ButtonManager& rhs) = delete;
                                    ButtonManager(ButtonManager&& rhs) = delete;
                                    ~ButtonManager() noexcept;

    ButtonManager&                  operator=(const ButtonManager& rhs) = delete;
    ButtonManager&                  operator=(ButtonManager&& rhs) = delete;

    bool                            run() noexcept override;

private:
    std::map<ButtonId_t, GPIOPin&>  buttons_;
    std::map<ButtonId_t, bool>      buttonStates_;
};

#endif // PERIPHERALS_BUTTONMANAGER_H_INC

