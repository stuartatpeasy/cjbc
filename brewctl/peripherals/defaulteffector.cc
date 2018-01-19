/*
    defaulteffector.cc: represents the absence of an effector; operations performed on objects of this type have no
    effect.  Used in cases where ptrs/refs to an EffectorInterface-derived object are required, but a real effector may
    not be present.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/peripherals/defaulteffector.h"
#include "include/framework/log.h"


DefaultEffector::DefaultEffector(const int channel, const std::string& name, const double powerConsumption) noexcept
    : channel_(channel),
      name_(name),
      state_(false),
      powerConsumption_(powerConsumption)
{
}


DefaultEffector::~DefaultEffector() noexcept
{
}


// activate() - do nothing, successfully.
//
bool DefaultEffector::activate(const bool state, Error * const err) noexcept
{
    // Suppress warnings about unused arguments
    (void) state;
    (void) err;

    logWarning("DefaultEffector::activate(): channel %d -> %s (nop)", channel_, state ? "on" : "off");

    return true;
}

