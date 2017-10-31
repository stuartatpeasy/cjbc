/*
    temperature.cc: declares class Temperature, which provides a unit-agnostic way of working with
    temperatures.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "temperature.h"
#include <cstdlib>

using std::string;

const double Temperature::zeroCelsiusInKelvin = 273.15;
const double Temperature::zeroFahrenheitInKelvin = 255.372;
const double Temperature::kelvinPerDegreeCelsius = 1.0;
const double Temperature::kelvinPerDegreeFahrenheit = 0.556;


// default ctor - construct an object representing the temperature absolute zero.
//
Temperature::Temperature()
    : valKelvin_(0.0)
{
}


Temperature::Temperature(const double value, const TemperatureUnit_t unit)
{
    set(value, unit);
}


bool Temperature::set(const double value, const TemperatureUnit_t unit)
{
    switch(unit)
    {
        case TEMP_UNIT_CELSIUS:
            if(value < -zeroCelsiusInKelvin)
                return false;       // Temperature is less than absolute zero
            valKelvin_ = (value * kelvinPerDegreeCelsius) + zeroCelsiusInKelvin;
            break;

        case TEMP_UNIT_FAHRENHEIT:
            if(value < -zeroFahrenheitInKelvin)
                return false;       // Temperature is less than absolute zero
            valKelvin_ = (value * kelvinPerDegreeFahrenheit) + zeroFahrenheitInKelvin;
            break;

        case TEMP_UNIT_KELVIN:
            if(value < 0)
                return false;       // Temperature is less than absolute zero
            valKelvin_ = value;
            break;

        default:
            return false;           // Unknown temperature unit
    }

    return true;
}


// C() - return a double representing this object's temperature in degrees Celsius.
double Temperature::C() const
{
    return (valKelvin_ - zeroCelsiusInKelvin) / kelvinPerDegreeCelsius;
}


// F() - return a double representing this object's temperature in degrees Fahrenheit.
//
double Temperature::F() const
{
    return (valKelvin_ - zeroFahrenheitInKelvin) / kelvinPerDegreeFahrenheit;
}


// K() - return a double representing this object's temperature in Kelvin.
//
double Temperature::K() const
{
    return valKelvin_;
}


// fromString() - attempt to parse a string containing a temperature (expressed as a number
// understood by strtod()) followed by a "unit suffix" (e.g. "C", "F") into the Temperature object
// <t>.  Note that this function will fail for strings containing a "degree" symbol (i.e. a
// superscript 'o').
//
bool Temperature::fromString(const string& s, Temperature& t)
{
    size_t endptr;

    const double val = stod(s, &endptr);
    TemperatureUnit_t unit = Temperature::unitFromChar(s[endptr]);

    if(!endptr || (unit == TEMP_UNIT_UNKNOWN))
        return false;       // No conversion was done, or unit-suffix is invalid

    return t.set(val, unit);
}


// fromString() - return the TemperatureUnit_t corresponding to the specified "unit suffix"
// character, e.g. 'C' -> TEMP_UNIT_CELSIUS.
//
TemperatureUnit_t Temperature::unitFromChar(const char c)
{
    switch(c)
    {
        case 'c':
        case 'C':
            return TEMP_UNIT_CELSIUS;

        case 'f':
        case 'F':
            return TEMP_UNIT_FAHRENHEIT;

        case 'k':
        case 'K':
            return TEMP_UNIT_KELVIN;

        default:
            return TEMP_UNIT_UNKNOWN;
    }
}

