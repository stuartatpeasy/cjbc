/*
    temperature.cc: declares class Temperature, which provides a unit-agnostic way of working with temperatures.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/application/temperature.h"
#include <cstdlib>


using std::string;

const double Temperature::tempTolerance             = 0.0001;   // Tolerance used when comparing temperatures

const double Temperature::zeroCelsiusInKelvin       = 273.15;
const double Temperature::zeroFahrenheitInKelvin    = 255.372;
const double Temperature::kelvinPerDegreeCelsius    = 1.0;
const double Temperature::kelvinPerDegreeFahrenheit = 0.556;


// default ctor - construct an object representing the temperature absolute zero.
//
Temperature::Temperature() noexcept
    : valKelvin_(0.0)
{
}


Temperature::Temperature(const double value, const TemperatureUnit_t unit) noexcept
{
    set(value, unit);
}


// get() - get the temperature represented by this object, in units specified by <unit>.
//
double Temperature::get(const TemperatureUnit_t unit) const noexcept
{
    switch(unit)
    {
        case TEMP_UNIT_CELSIUS:
            return (valKelvin_ - zeroCelsiusInKelvin) / kelvinPerDegreeCelsius;

        case TEMP_UNIT_FAHRENHEIT:
            return (valKelvin_ - zeroFahrenheitInKelvin) / kelvinPerDegreeFahrenheit;

        case TEMP_UNIT_KELVIN:
            return valKelvin_;

        default:
            return 0.0;             // Should be unreachable
    }
}


// set() - set the temperature represented by this object to <value>, specified in <unit> units.
//
bool Temperature::set(const double value, const TemperatureUnit_t unit) noexcept
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
//
double Temperature::C() const noexcept
{
    return get(TEMP_UNIT_CELSIUS);
}


// F() - return a double representing this object's temperature in degrees Fahrenheit.
//
double Temperature::F() const noexcept
{
    return get(TEMP_UNIT_FAHRENHEIT);
}


// K() - return a double representing this object's temperature in Kelvin.
//
double Temperature::K() const noexcept
{
    return get(TEMP_UNIT_KELVIN);
}


// fromString() - attempt to parse a string containing a temperature (expressed as a number understood by strtod())
// followed by a "unit suffix" (e.g. "C", "F") into the Temperature object <t>.  Note that this function will fail for
// strings containing a "degree" symbol (i.e. a superscript 'o').
//
bool Temperature::fromString(const string& s, Temperature& t) noexcept
{
    size_t endptr;

    const double val = stod(s, &endptr);
    TemperatureUnit_t unit = Temperature::unitFromChar(s[endptr]);

    if(!endptr || (unit == TEMP_UNIT_UNKNOWN))
        return false;       // No conversion was done, or unit-suffix is invalid

    return t.set(val, unit);
}


// fromString() - return the TemperatureUnit_t corresponding to the specified "unit suffix" character, e.g.
// 'C' -> TEMP_UNIT_CELSIUS.
//
TemperatureUnit_t Temperature::unitFromChar(const char c) noexcept
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


// diff() - return the difference between this object's temperature and the temperature represented by <rhs>, in units
// of <unit>.
//
double Temperature::diff(const Temperature& rhs, const TemperatureUnit_t unit) const noexcept
{
    return get(unit) - rhs.get(unit);
}

