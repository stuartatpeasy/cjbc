#ifndef THERMISTOR_H_INC
#define THERMISTOR_H_INC
/*
    thermistor.h: models an NTC thermistor using the beta parameter.  Deals with temperatures in
    Celsius.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include <cmath>
#include "temperature.h"


class Thermistor
{
public:
                    Thermistor(const double beta, const double R0, const double T0);
    virtual         ~Thermistor();

    Temperature     T(const double R);
    double          R(const Temperature& T);

protected:
    const double    beta_;
    const double    R0_;
    const double    T0_;

    double          Rinf_;
};

#endif // THERMISTOR_H_INC

