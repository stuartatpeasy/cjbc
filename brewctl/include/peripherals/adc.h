#ifndef PERIPHERALS_ADC_H_INC
#define PERIPHERALS_ADC_H_INC
/*
    adc.h: MCP3008 8-channel ADC driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/config.h"
#include "include/peripherals/gpioport.h"
#include <mutex>


typedef unsigned int ADCChannel_t;

class ADC
{
public:
                    ADC(GPIOPort& gpio, Config& config, Error * const err = nullptr) noexcept;

    double          read(const ADCChannel_t channel, Error * const err = nullptr) noexcept;
    double          vref() const noexcept { return vref_; };
    double          isource() const noexcept { return isource_; };

protected:
    double          vref_;
    double          isource_;
    bool            ready_;
    std::mutex      lock_;
};

#endif // PERIPHERALS_ADC_H_INC

