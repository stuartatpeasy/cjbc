#ifndef PERIPHERALS_DEFAULTEFFECTOR_H_INC
#define PERIPHERALS_DEFAULTEFFECTOR_H_INC
/*
    defaulteffector.h: represents the absence of an effector; operations performed on objects of this type have no
    effect.  Used in cases where ptrs/refs to an Effector object are required, but a real effector may not be present.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/framework/error.h"
#include <memory>
#include <string>


class DefaultEffector
{
public:
                                DefaultEffector(const int channel = -1, const std::string& name = "DefaultEffector")
                                    noexcept;
    virtual                     ~DefaultEffector() noexcept;

    virtual bool                activate(const bool state, Error * const err = nullptr) noexcept;
    virtual const std::string&  name() const noexcept { return name_; };
    int                         channel() const noexcept { return channel_; };

protected:
    int                         channel_;
    std::string                 name_;
};


typedef std::unique_ptr<DefaultEffector> DefaultEffector_uptr_t;


#endif // PERIPHERALS_DEFAULTEFFECTOR_H_INC

