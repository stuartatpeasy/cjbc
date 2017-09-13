#ifndef TEMPSENSOR_H_INC
#define TEMPSENSOR_H_INC
/*
    tempsensor.h: models a temperature sensor based on a NTC thermistor driven by a constant-
    current source, and an ADC channel.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "device.h"
#include "adc.h"
#include "thermistor.h"
#include "temperature.h"


class TempSensor : public Device
{
public:
                            TempSensor(Thermistor& thermistor, ADC& adc, const int channel,
                                       const double Idrive);
    virtual                 ~TempSensor();
    bool                    sense(Temperature& T);

protected:
    Thermistor&             thermistor_;
    ADC&                    adc_;
    int                     channel_;
    double                  Idrive_;
};


#endif // TEMPSENSOR_H_INC
