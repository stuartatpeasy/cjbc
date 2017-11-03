#ifndef EFFECTORINTERFACE_H_INC
#define EFFECTORINTERFACE_H_INC
/*
    effectorinterface.h: defines the interface to effectors, and provides some common methods.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "error.h"
#include <memory>
#include <string>


class EffectorInterface
{
protected:
                                EffectorInterface(const int channel, const double powerConsumption,
                                                  const std::string& name) noexcept;
public:
    virtual                     ~EffectorInterface() noexcept;

    virtual bool                activate(const bool state, Error * const err = nullptr) noexcept = 0;
    virtual bool                state() const noexcept { return state_; };
    virtual double              powerConsumption() const noexcept { return powerConsumption_; };
    virtual const std::string&  name() const noexcept { return name_; };

protected:
    const int                   channel_;
    const double                powerConsumption_;
    const std::string           name_;

    bool                        state_;
};


#endif // EFFECTORINTERFACE_H_INC
