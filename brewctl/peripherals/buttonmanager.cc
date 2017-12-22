/*
    buttonmanager.cc: operates a thread which monitors button inputs.

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/peripherals/buttonmanager.h"
#include "include/framework/log.h"
#include <utility>

extern "C"
{
#include <unistd.h>
}

using std::lock_guard;
using std::mutex;
using std::pair;
using std::vector;


const unsigned int BUTTON_READ_INTERVAL_MS = 10;        // Interval, in ms, between reads of the buttons

ButtonManager * ButtonManager::instance_ = nullptr;


// ctor - initialise map of <ButtonId_t> -> <GPIOPin&>
//
ButtonManager::ButtonManager() noexcept
    : Thread(),
      invalidButton_(Button::invalid_button)
{
}


// instance() - return an instance of the ButtonManager singleton
//
ButtonManager * ButtonManager::instance() noexcept
{
    if(instance_ == nullptr)
        instance_ = new ButtonManager;

    return instance_;
}


// run() - main loop.  Read buttons at a regular interval.
//
bool ButtonManager::run() noexcept
{
    running_ = true;
    setName("btns");

    while(!stop_)
    {
        update();
        ::usleep(BUTTON_READ_INTERVAL_MS * 1000);
    }

    running_ = false;

    return true;
}


// registerButton() - add the button identified by <button> to the map of buttons managed by this ButtonManager.
//
ButtonManager& ButtonManager::registerButton(const ButtonId_t button) noexcept
{
    buttons_.emplace(button, button);

    return *this;
}


// button() - retrieve a ref to the button specified by <button>.  If no such button exists, return a ref to the
// invalidButton member.
//
Button& ButtonManager::button(const ButtonId_t button) noexcept
{
    auto itButton = buttons_.find(button);

    return (itButton != buttons_.end()) ? itButton->second : invalidButton_;
}


// update() - read the state of all buttons; update buttonStates_ to reflect the new states.
//
void ButtonManager::update() noexcept
{
    lock_guard<mutex> lock(lock_);

    for(auto& button : buttons_)
        button.second.update();
}

