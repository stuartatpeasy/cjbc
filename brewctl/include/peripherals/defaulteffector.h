#ifndef PERIPHERALS_DEFAULTEFFECTOR_H_INC
#define PERIPHERALS_DEFAULTEFFECTOR_H_INC
/*
    defaulteffector.h: represents the absence of an effector; operations performed on objects of this type have no
    effect.  Used in cases where ptrs/refs to an Effector object are required, but a real effector may not be present.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/framework/error.h"
#include <ctime>
#include <memory>
#include <string>


class DefaultEffector
{
public:
                                DefaultEffector(const int channel = -1, const std::string& name = "DefaultEffector",
                                                const double powerConsumption = 0.0)
                                    noexcept;
    virtual                     ~DefaultEffector() noexcept = default;

    virtual bool                activate(const bool state, Error * const err = nullptr) noexcept;
    virtual const std::string&  name() const noexcept { return name_; };
    int                         channel() const noexcept { return channel_; };
    virtual bool                state() const noexcept { return state_; };
    virtual double              powerConsumption() const noexcept { return powerConsumption_; };
    virtual time_t              lastActivationTime() const noexcept { return lastActivationTime_; };
    virtual time_t              lastDeactivationTime() const noexcept { return lastDeactivationTime_; };
    virtual time_t              lastActiveDuration() const noexcept;
    virtual bool                logState(Error * const err = nullptr) const noexcept;

protected:
    int                         channel_;
    std::string                 name_;
    bool                        state_;
    double                      powerConsumption_;
    time_t                      lastActivationTime_;
    time_t                      lastDeactivationTime_;
    bool                        enabled_;
};


typedef std::unique_ptr<DefaultEffector> DefaultEffector_uptr_t;


#endif // PERIPHERALS_DEFAULTEFFECTOR_H_INC

