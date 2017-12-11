#ifndef APPLICATION_TEMPERATURE_H_INC
#define APPLICATION_TEMPERATURE_H_INC
/*
    temperature.h: declares class Temperature, which provides a unit-agnostic way of working with temperatures.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include <cmath>        // ::fabs()
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
                                Temperature() noexcept;
                                Temperature(const double value, const TemperatureUnit_t unit) noexcept;

    double                      get(const TemperatureUnit_t unit) const noexcept;
    bool                        set(const double value, const TemperatureUnit_t unit) noexcept;

    double                      C() const noexcept;
    double                      F() const noexcept;
    double                      K() const noexcept;

    static bool                 fromString(const std::string& s, Temperature& t) noexcept;
    static TemperatureUnit_t    unitFromChar(const char c) noexcept;

    double                      diff(const Temperature& rhs, const TemperatureUnit_t unit) const noexcept;

    bool                        operator==(const Temperature& rhs) const noexcept
                                {
                                    return ::fabs(valKelvin_ - rhs.valKelvin_) < tempTolerance;
                                };

    bool                        operator!=(const Temperature& rhs) const noexcept
                                {
                                    return ::fabs(valKelvin_ - rhs.valKelvin_) >= tempTolerance;
                                };

    bool                        operator!() const noexcept
                                {
                                    return valKelvin_ == 0.0;
                                };

                                operator bool() const noexcept
                                {
                                    return valKelvin_ != 0.0;
                                };

    bool                        operator<(const Temperature& rhs) const noexcept
                                {
                                    return (rhs.valKelvin_ - valKelvin_) >= tempTolerance;
                                };

    bool                        operator<=(const Temperature& rhs) const noexcept
                                {
                                    return operator<(rhs) || operator==(rhs);
                                };

    bool                        operator>(const Temperature& rhs) const noexcept
                                {
                                    return (valKelvin_ - rhs.valKelvin_) >= tempTolerance;
                                };

    bool                        operator>=(const Temperature& rhs) const noexcept
                                {
                                    return operator>(rhs) || operator==(rhs);
                                };

    Temperature                 operator+(const Temperature& rhs) const noexcept
                                {
                                    return Temperature(valKelvin_ + rhs.valKelvin_, TEMP_UNIT_KELVIN);
                                };

    Temperature&                operator+=(const Temperature& rhs) noexcept
                                {
                                    valKelvin_ += rhs.valKelvin_;
                                    return *this;
                                };

    Temperature                 operator-(const Temperature& rhs) const noexcept
                                {
                                    return Temperature(((*this > rhs) ? (valKelvin_ - rhs.valKelvin_) : 0.0),
                                                       TEMP_UNIT_KELVIN);
                                };

    Temperature&                operator-=(const Temperature& rhs) noexcept
                                {
                                    valKelvin_ -= rhs.valKelvin_;
                                    if(valKelvin_ < 0.0)
                                        valKelvin_ = 0.0;
                                    return *this;
                                };

protected:
    double                      valKelvin_;

    static const double         tempTolerance;

    static const double         zeroCelsiusInKelvin;
    static const double         zeroFahrenheitInKelvin;
    static const double         kelvinPerDegreeCelsius;
    static const double         kelvinPerDegreeFahrenheit;
};

#endif // APPLICATION_TEMPERATURE_H_INC

