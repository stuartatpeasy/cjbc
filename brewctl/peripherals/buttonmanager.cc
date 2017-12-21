/*
    buttonmanager.cc: operates a thread which monitors button inputs.

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/peripherals/buttonmanager.h"
#include "include/framework/log.h"
#include "include/framework/registry.h"
#include <utility>

extern "C"
{
#include <unistd.h>
}

using std::pair;
using std::vector;


const unsigned int BUTTON_READ_INTERVAL_MS = 10;        // Interval, in ms, between reads of the buttons


static vector<ButtonId_t> buttons =
{
    SWITCH_BOTTOM, SWITCH_TOP, ROT_CW, ROT_CCW, ROT_BUTTON
};


// ctor - initialise map of <ButtonId_t> -> <GPIOPin&>
//
ButtonManager::ButtonManager() noexcept
    : Thread()
{
    auto& gpioport = Registry::instance().gpio();

    for(auto button : buttons)
    {
        buttons_.insert(pair<ButtonId_t, GPIOPin&>(button, gpioport.pin(button)));
        buttonStates_[button] = false;
    }
}


// dtor
//
ButtonManager::~ButtonManager() noexcept
{
}


// run() - main loop.  Read buttons at a regular interval.
//
bool ButtonManager::run() noexcept
{
    running_ = true;
    setName("btns");

    while(!stop_)
    {
        for(auto it : buttons_)
        {
            const bool state = it.second.read();

            if(state != buttonStates_[it.first])
                logDebug("Button %d %s", it.first, state ? "pressed" : "released");

            buttonStates_[it.first] = state;
        }

        ::usleep(BUTTON_READ_INTERVAL_MS * 1000);
    }

    running_ = false;

    return true;
}

