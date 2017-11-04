/*
    nulleffector.cc: represents the absence of an effector; operations performed on objects of this type have no effect.
    Used in cases where ptrs/refs to an EffectorInterface-derived object are required, but a real effector may not be
    present.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "nulleffector.h"


NullEffector::NullEffector(const int channel) noexcept
    : Effector(channel, 0.0, "NullEffector")
{
}


NullEffector::~NullEffector()
{
}


// activate() - do nothing, successfully.
//
bool NullEffector::activate(const bool state, Error * const err) noexcept
{
    // Suppress warnings about unused arguments
    (void) state;
    (void) err;

    return true;
}
