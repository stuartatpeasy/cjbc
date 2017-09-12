#ifndef TEMPERATURE_H_INC
#define TEMPERATURE_H_INC
/*
    temperature.h: declares class Temperature, which provides a unit-agnostic way of working with
    temperatures.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/


typedef enum TemperatureUnit
{
    TEMP_UNIT_CELSIUS,
    TEMP_UNIT_FAHRENHEIT,
    TEMP_UNIT_KELVIN
} TemperatureUnit_t;


class Temperature
{
public:
                    Temperature(const double value, const TemperatureUnit_t unit);
    virtual         ~Temperature();

    double          C();
    double          F();
    double          K();

protected:
    double          valKelvin_;

    static const double zeroCelsiusInKelvin;
    static const double zeroFahrenheitInKelvin;
    static const double kelvinPerDegreeCelsius;
    static const double kelvinPerDegreeFahrenheit;
};

#endif // TEMPERATURE_H_INC

