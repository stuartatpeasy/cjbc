/*
    defaulteffector.cc: represents the absence of an effector; operations performed on objects of this type have no
    effect.  Used in cases where ptrs/refs to an EffectorInterface-derived object are required, but a real effector may
    not be present.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/peripherals/defaulteffector.h"
#include "include/framework/log.h"
#include "include/framework/registry.h"
#include "include/util/validator.h"

namespace Validator = Util::Validator;


DefaultEffector::DefaultEffector(const int channel, const std::string& name, const double powerConsumption) noexcept
    : channel_(channel),
      name_(name),
      state_(false),
      powerConsumption_(powerConsumption),
      lastActivationTime_(0),
      lastDeactivationTime_(0)
{
    enabled_ = !Registry::instance().config().get("application.disable_effectors", 0, Validator::ge0);
}


// activate() - do nothing, successfully.
//
bool DefaultEffector::activate(const bool state, Error * const err) noexcept
{
    // Suppress warnings about unused arguments
    (void) state;
    (void) err;

    logWarning("DefaultEffector::activate(): channel %d -> %s (nop)", channel_, state ? "on" : "off");

    return true;
}


// lastActiveDuration() - return the length of time, in seconds, of the last activation of the effector.  If the
// effector has never been activated, or has been activated but not deactivated, returns 0.
//
time_t DefaultEffector::lastActiveDuration() const noexcept
{
    return (lastActivationTime_ && lastDeactivationTime_) ? lastDeactivationTime_ - lastActivationTime_ : 0;
}


// logState() - log the effector's current state in the "effectorlog" database table.  Returns true on success, false
// otherwise.
//
bool DefaultEffector::logState(Error * const err) const noexcept
{
    SQLiteStmt effectorLog;

    if(!Registry::instance().db().prepare("INSERT INTO effectorlog(effector_id, newstate) "
                                          "VALUES(:effectorId, :newState)", effectorLog, err) ||
       !effectorLog.bind(":effectorId", channel_, err) ||
       !effectorLog.bind(":newState", state_, err) ||
       !effectorLog.execute(err))
    {
        logWarning("Failed to log effector %d transition to state '%s'", channel_, state_ ? "on" : "off");
        return false;
    }

    return true;
}

