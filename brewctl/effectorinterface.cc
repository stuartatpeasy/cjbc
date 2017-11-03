/*
    effectorinterface.cc: defines the interface to effectors, and provides some common methods.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "effectorinterface.h"

using std::string;


EffectorInterface::EffectorInterface(const int channel, const double powerConsumption, const string& name) noexcept
    : channel_(channel), powerConsumption_(powerConsumption), name_(name), state_(false)
{
}


EffectorInterface::~EffectorInterface()
{
}

