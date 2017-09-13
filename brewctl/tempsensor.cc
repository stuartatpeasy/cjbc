/*
    tempsensor.h: models a temperature sensor based on a NTC thermistor driven by a constant-
    current source, and an ADC channel.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "tempsensor.h"


TempSensor::TempSensor(Thermistor& thermistor, ADC& adc, const int channel, const double Idrive)
    : Device(), thermistor_(thermistor), adc_(adc), channel_(channel), Idrive_(Idrive)
{

}


TempSensor::~TempSensor()
{
}


bool TempSensor::sense(Temperature& T)
{
    double adcVoltage = 0.0;

    if(!adc_.read(channel_, adcVoltage))
        return false;

    T = thermistor_.T(adcVoltage / Idrive_);

    return true;
}

