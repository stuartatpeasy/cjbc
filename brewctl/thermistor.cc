/*
    thermistor.cc: models an NTC thermistor using the beta parameter.  Deals with temperatures in
    Celsius.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "thermistor.h"


Thermistor::Thermistor(const double beta, const double R0, const double T0)
    : beta_(beta), R0_(R0), T0_(T0)
{
    // TODO: ensure T0 != 0.0

    Rinf_ = R0_ * exp(-beta_ / T0_);
}


Thermistor::~Thermistor()
{
}


// T() - return the temperature corresponding to the resistance R, given in ohms.
//
Temperature Thermistor::T(const double R)
{
    return Temperature(0.0, TEMP_UNIT_KELVIN);
}


// R() - return the resistance corresponding to the temperature T.
//
double Thermistor::R(const Temperature& T)
{
    return 0.0;
}
