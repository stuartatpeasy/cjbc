#ifndef TEMPERATURE_H_INC
#define TEMPERATURE_H_INC
/*
    temperature.h: declares class Temperature, which provides a unit-agnostic way of working with
    temperatures.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include <string>


typedef enum TemperatureUnit
{
    TEMP_UNIT_CELSIUS,
    TEMP_UNIT_FAHRENHEIT,
    TEMP_UNIT_KELVIN,
    TEMP_UNIT_UNKNOWN
} TemperatureUnit_t;


class Temperature
{
public:
                                Temperature();
                                Temperature(const double value, const TemperatureUnit_t unit);

    bool                        set(const double value, const TemperatureUnit_t unit);

    double                      C() const;
    double                      F() const;
    double                      K() const;

    static bool                 fromString(const std::string& s, Temperature& t);
    static TemperatureUnit_t    unitFromChar(const char c);

protected:
    double                      valKelvin_;

    static const double         zeroCelsiusInKelvin;
    static const double         zeroFahrenheitInKelvin;
    static const double         kelvinPerDegreeCelsius;
    static const double         kelvinPerDegreeFahrenheit;
};

#endif // TEMPERATURE_H_INC

