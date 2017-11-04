#ifndef NULLEFFECTOR_H_INC
#define NULLEFFECTOR_H_INC
/*
    nulleffector.h: represents the absence of an effector; operations performed on objects of this type have no effect.
    Used in cases where ptrs/refs to an Effector object are required, but a real effector may not be present.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "effector.h"
#include <memory>
#include <string>


class NullEffector : public Effector
{
public:
                    NullEffector(const int channel = -1) noexcept;
    virtual         ~NullEffector() noexcept;

    virtual bool    activate(const bool state, Error * const err = nullptr) noexcept;
};


#endif // NULLEFFECTOR_H_INC

