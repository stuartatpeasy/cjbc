/*
    tempsensor.cc: models a temperature sensor based on a NTC thermistor driven by a constant-
    current source, and an ADC channel.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "tempsensor.h"
#include "registry.h"


TempSensor::TempSensor(Thermistor& thermistor, const int channel, const double Idrive)
    : thermistor_(thermistor), channel_(channel), Idrive_(Idrive), nsamples_(1000),
      tempKelvin_(0.0), sampleTaken_(false)
{
}


TempSensor& TempSensor::setMovingAvgLen(const unsigned int len)
{
    if(len)
        nsamples_ = len;

    return *this;
}


bool TempSensor::sense(Temperature& T)
{
    double adcVoltage = 0.0;

    if(!Registry::instance().adc().read(channel_, adcVoltage))
        return false;

    const Temperature sample = thermistor_.T(adcVoltage / Idrive_);

    // If this is the first sample, set the moving-average value to the sampled temperature in order
    // to initialise it to an approximate value.  If this is not the first sample, use the data to
    // adjust the moving average value.
    if(sampleTaken_)
    {
        // At least one sample has already been taken
        tempKelvin_ -= tempKelvin_ / nsamples_;
        tempKelvin_ += sample.K() / nsamples_;
    }
    else
    {
        // This is the first sample
        tempKelvin_ = sample.K();
        sampleTaken_ = true;
    }

    T.set(tempKelvin_, TEMP_UNIT_KELVIN);

    return true;
}

