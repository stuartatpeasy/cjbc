#ifndef NULLEFFECTOR_H_INC
#define NULLEFFECTOR_H_INC
/*
    nulleffector.h: represents the absence of an effector; operations performed on objects of this type have no effect.
    Used in cases where ptrs/refs to an EffectorInterface-derived object are required, but a real effector may not be
    present.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "effectorinterface.h"
#include <memory>
#include <string>


class NullEffector : public EffectorInterface
{
public:
            NullEffector() noexcept;

    bool    activate(const bool state, Error * const err = nullptr) noexcept;
};


#endif // NULLEFFECTOR_H_INC

