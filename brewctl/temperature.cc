/*
    temperature.cc: declares class Temperature, which provides a unit-agnostic way of working with
    temperatures.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "temperature.h"


const double Temperature::zeroCelsiusInKelvin = 273.15;
const double Temperature::zeroFahrenheitInKelvin = 255.372;
const double Temperature::kelvinPerDegreeCelsius = 1.0;
const double Temperature::kelvinPerDegreeFahrenheit = 0.556;


Temperature::Temperature()
    : valKelvin_(0.0)
{
}


Temperature::Temperature(const double value, const TemperatureUnit_t unit)
{
    switch(unit)
    {
        case TEMP_UNIT_CELSIUS:
            valKelvin_ = (value * kelvinPerDegreeCelsius) + zeroCelsiusInKelvin;
            break;

        case TEMP_UNIT_FAHRENHEIT:
            valKelvin_ = (value * kelvinPerDegreeFahrenheit) + zeroFahrenheitInKelvin;
            break;

        case TEMP_UNIT_KELVIN:
            valKelvin_ = value;
            break;
    }
}


Temperature::~Temperature()
{
}


double Temperature::C() const
{
    return (valKelvin_ - zeroCelsiusInKelvin) / kelvinPerDegreeCelsius;
}


double Temperature::F() const
{
    return (valKelvin_ - zeroFahrenheitInKelvin) / kelvinPerDegreeFahrenheit;
}


double Temperature::K() const
{
    return valKelvin_;
}

