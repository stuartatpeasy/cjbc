/*
    adc.h: MCP3008 8-channel ADC driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "adc.h"
#include <cerrno>

#define ADC_MAX_CHANNEL     (7)         // This ADC has channels numbered 0-7.
#define ADC_BITS            (10)        // This ADC has 10-bit resolution.

#define ADC_START_BIT       (1 << 0)
#define ADC_SINGLE_MODE_BIT (1 << 7)
#define ADC_CHANNEL_SHIFT   (4)
#define ADC_PACKET_LEN      (3)         // An ADC read operation requires a 3-byte "packet"

//
// Mapping of GPIO pin names to wiringPi pin numbers for ADC pins
//
typedef enum ADCPin
{
    GPIO_ADC_nCS  = 1       // ADC chip select (active low)
} ADCPin_t;



ADC::ADC(GPIOPort& gpio, SPIPort& spi, const double& vref)
    : Device(), gpio_(gpio), spi_(spi), vref_(vref), ready_(false)
{
    // Set ADC_nCS as an output, and de-assert it
    gpio_.write(GPIO_ADC_nCS, 1);
    
    if(!gpio_.setMode(GPIO_ADC_nCS, PIN_OUTPUT))
        return;

    if(vref_ > 0.0)
        ready_ = true;
}


ADC::~ADC()
{
}


bool ADC::read(const unsigned int channel, double& voltage)
{
    uint8_t tx_data[ADC_PACKET_LEN], rx_data[ADC_PACKET_LEN];
    unsigned short val;

    if(!ready_)
    {
        errno_ = EAGAIN;
        return false;
    }

    if(channel > ADC_MAX_CHANNEL)
    {
        errno_ = EINVAL;
        return false;
    }

    gpio_.write(GPIO_ADC_nCS, 0);       // Assert the ADC's nCS line

    tx_data[0] = ADC_START_BIT;
    tx_data[1] = ADC_SINGLE_MODE_BIT | (channel << ADC_CHANNEL_SHIFT);
    tx_data[2] = 0;     // Don't care

    const bool ret = spi_.transmitAndReceive(tx_data, rx_data, ADC_PACKET_LEN);

    gpio_.write(GPIO_ADC_nCS, 1);       // Negate the ADC's nCS line

    if(ret)
    {
        val = ((rx_data[1] & 0x03) << 8) + rx_data[2];                      // Calculate raw value
        voltage = (double) val * vref_ / (double) ((1 << ADC_BITS) - 1);    // Convert to voltage
        errno_ = 0;
    }
    else
    {
        errno_ = spi_.errNo();
        spi_.resetErrNo();
    }

    return ret;
}

