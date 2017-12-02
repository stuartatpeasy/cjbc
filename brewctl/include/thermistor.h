#ifndef THERMISTOR_H_INC
#define THERMISTOR_H_INC
/*
    thermistor.h: models an NTC thermistor using the beta parameter.  Deals with temperatures in Celsius.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include <cmath>
#include "temperature.h"


class Thermistor
{
public:
                        Thermistor(const double beta, const double R0, const Temperature& T0) noexcept;

    Temperature         T(const double R) const noexcept;
    double              R(const Temperature& T) const noexcept;

protected:
    const double        beta_;
    const double        R0_;
    const Temperature   T0_;

    double              Rinf_;
};

#endif // THERMISTOR_H_INC

