/*
    effector.cc: effector abstraction.  Provides an interface to switch on and off a device attached to a particular
    output channel.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/framework/log.h"
#include "include/framework/registry.h"
#include "include/peripherals/effector.h"
#include "include/sqlite/sqlite.h"
#include "include/sqlite/sqlitestmt.h"

using std::string;


static const int EFFECTOR_BIT_OFFSET    = 8;    // Offset of effector bit 0 from shift register bit 0


Effector::Effector(const int channel, const double powerConsumption, const string& name) noexcept
    : DefaultEffector(channel, name, powerConsumption)
{
    logDebug("instantiating effector on channel %d with power consumption %.3fW", channel, powerConsumption);
}


Effector::~Effector() noexcept
{
}


// move ctor
//
Effector::Effector(Effector&& rhs) noexcept
{
    move(rhs);
}


// move-assignment operator
//
Effector& Effector::operator=(Effector&& rhs) noexcept
{
    move(rhs);
    return *this;
}


// move() - helper method used by move ctor / move-assignment operator
//
void Effector::move(Effector& rhs) noexcept
{
    channel_ = rhs.channel_;
    powerConsumption_ = rhs.powerConsumption_;
    name_ = rhs.name_;
    state_ = rhs.state_;

    rhs.channel_ = -1;
    rhs.powerConsumption_ = 0.0;
    rhs.name_ = "";
    rhs.state_ = false;
}


// activate() - set the "active set" (i.e. 0 or 1) or this effector's output; i.e. switch the effector on or off,
// depending on the value of <state>.  Returns true on success; false otherwise.
//
bool Effector::activate(const bool state, Error * const err) noexcept
{
    auto& sr = Registry::instance().sr();
    const int bit = EFFECTOR_BIT_OFFSET + channel_;
    const bool stateChanged = (state != state_);

    // If this is a change of state, log it
    if(stateChanged)
        logDebug("activate(): channel %d -> %s%s", channel_, state ? "on" : "off",
                 enabled_ ? "" : " (effectors disabled)");

    // Attempt to flip the shift register bit corresponding to this effector
    if(enabled_ && !(state ? sr.set(bit, err) : sr.clear(bit, err)))
        return false;

    // Update local record of activation / deactivation time
    if(state)
        lastActivationTime_ = ::time(NULL);
    else
        lastDeactivationTime_ = ::time(NULL);

    state_ = state;

    if(stateChanged)
        logState();

    return true;
}


// getSessionEffectorByType() - factory for Effector objects.  Instantiates an Effector of the given type, for the
// given session.  If there is no matching effector in the database, return a new DefaultEffector.  If the database
// lookup fails, a nullptr is returned.  All values a returned through a DefaultEffector_uptr_t type (a
// std::unique_ptr<DefaultEffector *>).
//
DefaultEffector_uptr_t Effector::getSessionEffectorByType(const int sessionId, const std::string& type,
                                                          Error * const err) noexcept
{
    SQLiteStmt eff;
    DefaultEffector* ret;

    if(!Registry::instance().db().prepare("SELECT channel, name, powerconsumption FROM sessioneffector "
                                          "LEFT JOIN effectortype ON sessioneffector.effectortype_id=effectortype.id "
                                          "WHERE session_id=:sessionId AND type=:type", eff, err) ||
       !eff.bind(":sessionId", sessionId, err) ||
       !eff.bind(":type", type, err) ||
       !eff.step(err))
    {
        logInfo("Session %d: no effector of type '%s' found", sessionId, type.c_str());
        ret = new DefaultEffector();
    }
    else
        ret = new Effector(eff["channel"].get<int>(), eff["powerconsumption"].get<double>(), eff["name"].get<string>());

    if(ret == nullptr)
        formatError(err, MALLOC_FAILED);

    return DefaultEffector_uptr_t(ret);
}


// getSessionHeater() - factory for Effector objects of type "heater".  Instantiates an Effector of the "heater" type
// for the given session.
//
DefaultEffector_uptr_t Effector::getSessionHeater(const int sessionId, Error * const err) noexcept
{
    return getSessionEffectorByType(sessionId, "heater", err);
}


// getSessionCooler() - factory for Effector objects of type "cooler".  Instantiates an Effector of the "cooler" type
// for the given session.
//
DefaultEffector_uptr_t Effector::getSessionCooler(const int sessionId, Error * const err) noexcept
{
    return getSessionEffectorByType(sessionId, "cooler", err);
}

