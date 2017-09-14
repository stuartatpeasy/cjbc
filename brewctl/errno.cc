/*
    errno.cc: base class providing error-number tracking/discovery functions

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "errno.h"


ErrNo::ErrNo()
    : errno_(0)
{
}


ErrNo::~ErrNo()
{
}


