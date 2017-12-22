#ifndef PERIPHERALS_BUTTONMANAGER_H_INC
#define PERIPHERALS_BUTTONMANAGER_H_INC
/*
    buttonmanager.h: operates a thread which monitors button inputs.

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/peripherals/button.h"
#include "include/peripherals/gpioport.h"
#include "include/framework/thread.h"
#include <map>
#include <memory>
#include <mutex>


class ButtonManager : public Thread
{
private:
                                    ButtonManager() noexcept;
                                    ButtonManager(const ButtonManager& rhs) = delete;
                                    ButtonManager(ButtonManager&& rhs) = delete;
                                    ~ButtonManager() noexcept = default;

    ButtonManager&                  operator=(const ButtonManager& rhs) = delete;
    ButtonManager&                  operator=(ButtonManager&& rhs) = delete;

public:
    static ButtonManager *          instance() noexcept;
    bool                            run() noexcept override;
    ButtonManager&                  registerButton(const ButtonId_t button) noexcept;
    Button&                         button(const ButtonId_t button) noexcept;

private:
    void                            update() noexcept;

    static ButtonManager *          instance_;
    Button                          invalidButton_;
    std::map<ButtonId_t, Button>    buttons_;
    std::mutex                      lock_;
};

#endif // PERIPHERALS_BUTTONMANAGER_H_INC

