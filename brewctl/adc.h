#ifndef ADC_H_INC
#define ADC_H_INC
/*
    adc.h: MCP3008 8-channel ADC driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "config.h"
#include "gpioport.h"


class ADC
{
public:
                    ADC(GPIOPort& gpio, Config& config, Error * const err = nullptr);

    bool            read(const unsigned int channel, double& voltage, Error * const err = nullptr);
    double          vref() const { return vref_; };

protected:
    double          vref_;
    bool            ready_;
};

#endif // ADC_H_INC

