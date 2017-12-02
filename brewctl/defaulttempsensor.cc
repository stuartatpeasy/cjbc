/*
    defaulttempsensor.cc: representation of the absence of a temperature sensor.  Any temperature measurements taken
    through objects of this type will return absolute zero.  Useful in cases where a ref/ptr to a TempSensor is
    required, but no sensor actually exists.

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "include/defaulttempsensor.h"
#include "include/log.h"
#include "include/registry.h"


DefaultTempSensor::DefaultTempSensor(const int channel, const std::string& name) noexcept
    : channel_(channel),
      name_(name)
{
}


DefaultTempSensor::~DefaultTempSensor() noexcept
{
}


// copy ctor
//
DefaultTempSensor::DefaultTempSensor(const DefaultTempSensor& rhs) noexcept
{
    copy(rhs);
}


// move ctor
//
DefaultTempSensor::DefaultTempSensor(DefaultTempSensor&& rhs) noexcept
{
    move(rhs);
}


// copy-assignment operator
//
DefaultTempSensor& DefaultTempSensor::operator=(const DefaultTempSensor& rhs) noexcept
{
    copy(rhs);
    return *this;
}


// move-assignment operator
//
DefaultTempSensor& DefaultTempSensor::operator=(DefaultTempSensor&& rhs) noexcept
{
    move(rhs);
    return *this;
}


// sense() - always return a temperature corresponding to absolute zero.
//
Temperature DefaultTempSensor::sense(Error * const err) noexcept
{
    (void) err;     // Suppress "unused arg" warning
    logDebug("DefaultTempSensor::sense() called for channel %d", channel_);
    return Temperature();
}


// move() - helper for move ctor / move-assignment operator
//
void DefaultTempSensor::move(DefaultTempSensor& rhs) noexcept
{
    channel_    = rhs.channel_;
    name_       = rhs.name_;

    rhs.channel_    = -1;
    rhs.name_       = "";
}


// copy() - helper for copy ctor / copy-assignment operator
//
void DefaultTempSensor::copy(const DefaultTempSensor& rhs) noexcept
{
    channel_ = rhs.channel_;
    name_ = rhs.name_;
}

