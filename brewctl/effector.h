#ifndef EFFECTOR_H_INC
#define EFFECTOR_H_INC
/*
    effector.h: effector abstraction.  Provides an interface to switch on and off a device attached to a particular
    output channel.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "error.h"
#include <memory>
#include <string>


class Effector
{
public:
                        Effector(const int channel, const double powerConsumption, const std::string& name) noexcept;

    bool                activate(const bool state, Error * const err = nullptr) noexcept;
    bool                state() const noexcept { return state_; };
    double              powerConsumption() const noexcept { return powerConsumption_; };
    const std::string&        name() const noexcept { return name_; };

    static Effector*    getSessionEffectorByType(const int sessionId, const std::string& type,
                                                 Error * const err = nullptr) noexcept;
    static Effector*    getSessionHeater(const int sessionId, Error * const err = nullptr) noexcept;
    static Effector*    getSessionCooler(const int sessionId, Error * const err = nullptr) noexcept;

private:
    const int           channel_;
    const double        powerConsumption_;
    const std::string   name_;

    bool                state_;
};

#endif // EFFECTOR_H_INC

