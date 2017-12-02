#ifndef PERIPHERALS_EFFECTOR_H_INC
#define PERIPHERALS_EFFECTOR_H_INC
/*
    effector.h: effector abstraction.  Provides an interface to switch on and off a device attached to a particular
    output channel.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/error.h"
#include "include/peripherals/defaulteffector.h"
#include <memory>
#include <string>


class Effector : public DefaultEffector
{
public:
                                    Effector(const int channel, const double powerConsumption, const std::string& name)
                                        noexcept;
    virtual                         ~Effector() noexcept;

                                    Effector(const Effector& rhs) = delete;
                                    Effector(Effector&& rhs) noexcept;

    Effector&                       operator=(const Effector& rhs) = delete;
    Effector&                       operator=(Effector&& rhs) noexcept;

    virtual bool                    activate(const bool state, Error * const err = nullptr) noexcept;

    virtual bool                    state() const noexcept { return state_; };
    virtual double                  powerConsumption() const noexcept { return powerConsumption_; };

    static DefaultEffector_uptr_t   getSessionHeater(const int sessionId, Error * const err = nullptr) noexcept;
    static DefaultEffector_uptr_t   getSessionCooler(const int sessionId, Error * const err = nullptr) noexcept;

protected:
    static DefaultEffector_uptr_t   getSessionEffectorByType(const int sessionId, const std::string& type,
                                                             Error * const err = nullptr) noexcept;
    void                            move(Effector& rhs) noexcept;

    double                          powerConsumption_;
    bool                            state_;
};

#endif // PERIPHERALS_EFFECTOR_H_INC

