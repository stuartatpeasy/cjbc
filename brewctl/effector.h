#ifndef EFFECTOR_H_INC
#define EFFECTOR_H_INC
/*
    effector.h: effector abstraction.  Provides an interface to switch on and off a device attached to a particular
    output channel.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "effectorinterface.h"
#include "error.h"
#include <memory>
#include <string>


class Effector : public EffectorInterface
{
public:
                                Effector(const int channel, const double powerConsumption, const std::string& name)
                                    noexcept;

    bool                        activate(const bool state, Error * const err = nullptr) noexcept;

    static EffectorInterface*   getSessionEffectorByType(const int sessionId, const std::string& type,
                                                         Error * const err = nullptr) noexcept;
    static EffectorInterface*   getSessionHeater(const int sessionId, Error * const err = nullptr) noexcept;
    static EffectorInterface*   getSessionCooler(const int sessionId, Error * const err = nullptr) noexcept;
};

#endif // EFFECTOR_H_INC

