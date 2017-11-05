/*
    effector.h: effector abstraction.  Provides an interface to switch on and off a device attached to a particular
    output channel.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "effector.h"
#include "log.h"
#include "nulleffector.h"
#include "registry.h"
#include "sqlite.h"
#include "sqlitestmt.h"

using std::string;


#define EFFECTOR_BIT_OFFSET     (8)     // Offset of effector bit 0 from shift register bit 0


Effector::Effector(const int channel, const double powerConsumption, const string& name) noexcept
    : channel_(channel), powerConsumption_(powerConsumption), name_(name), state_(false)
{
    logDebug("instantiating effector on channel %d with power consumption %f", channel, powerConsumption);
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

    logDebug("activate(): channel %d -> %s", channel_, state ? "on" : "off");

    if(!(state ? sr.set(bit, err) : sr.clear(bit, err)))
        return false;

    state_ = state;
    return true;
}


// getSessionEffectorByType() - factory for Effector objects.  Instantiates an Effector of the given type, for the
// given session.  If there is no matching effector in the database, return a new NullEffector.  If the database lookup
// fails, a nullptr is returned.
//
Effector* Effector::getSessionEffectorByType(const int sessionId, const std::string& type, Error * const err)
    noexcept
{
    auto& db = Registry::instance().db();
    SQLiteStmt eff;

    if(!db.prepare("SELECT channel, name, powerconsumption FROM sessioneffector "
                   "LEFT JOIN effectortype ON sessioneffector.effectortype_id=effectortype.id "
                   "WHERE session_id=:sessionId AND type=:type", eff, err) ||
       !eff.bind(":sessionId", sessionId, err) ||
       !eff.bind(":type", type, err))
        return nullptr;

    if(!eff.step(err))
        return err->code() ? nullptr : new NullEffector();

    // FIXME - verify that channel is in bounds
    return new Effector(eff["channel"], eff["powerconsumption"], eff["name"].asString());
}


// getSessionHeater() - factory for Effector objects of type "heater".  Instantiates an Effector of the "heater" type
// for the given session.
//
Effector* Effector::getSessionHeater(const int sessionId, Error * const err) noexcept
{
    return getSessionEffectorByType(sessionId, "heater", err);
}


// getSessionCooler() - factory for Effector objects of type "cooler".  Instantiates an Effector of the "cooler" type
// for the given session.
//
Effector* Effector::getSessionCooler(const int sessionId, Error * const err) noexcept
{
    return getSessionEffectorByType(sessionId, "cooler", err);
}

