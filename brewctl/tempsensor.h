#ifndef TEMPSENSOR_H_INC
#define TEMPSENSOR_H_INC
/*
    tempsensor.h: models a temperature sensor based on a NTC thermistor driven by a constant-
    current source, and an ADC channel.

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "thermistor.h"
#include "temperature.h"


class TempSensor
{
public:
                            TempSensor(Thermistor& thermistor, const int channel,
                                       const double Idrive);

    bool                    sense(Temperature& T);
    TempSensor&             setMovingAvgLen(const unsigned int len);
    unsigned int            getMovingAvgLen() const { return nsamples_; };

protected:
    Thermistor&             thermistor_;
    int                     channel_;
    double                  Idrive_;

    unsigned int            nsamples_;      // Length of moving average of samples
    double                  tempKelvin_;    // Current moving average val of temp in K
    bool                    sampleTaken_;   // Set once one sample has been taken
};


#endif // TEMPSENSOR_H_INC
