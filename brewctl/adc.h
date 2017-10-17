#ifndef ADC_H_INC
#define ADC_H_INC
/*
    adc.h: MCP3008 8-channel ADC driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "device.h"
#include "config.h"
#include "gpioport.h"
#include "spiport.h"


class ADC : public Device
{
public:
                    ADC(GPIOPort* gpio, SPIPort* spi, Config& config, Error * const err = nullptr);
    virtual         ~ADC();

    bool            read(const unsigned int channel, double& voltage, Error * const err = nullptr);
    double          vref() const { return vref_; };

protected:
    GPIOPort *      gpio_;
    SPIPort *       spi_;
    double          vref_;
    bool            ready_;
};

#endif // ADC_H_INC

