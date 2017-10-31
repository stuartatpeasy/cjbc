/*
    adc.cc: MCP3008 8-channel ADC driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "adc.h"
#include "registry.h"


#define ADC_MAX_CHANNEL         (7)         // This ADC has channels numbered 0-7.
#define ADC_BITS                (10)        // This ADC has 10-bit resolution.
#define ADC_DEFAULT_REF_VOLTAGE (5.0)       // Default ADC reference voltage

#define ADC_START_BIT           (1 << 0)
#define ADC_SINGLE_MODE_BIT     (1 << 7)
#define ADC_CHANNEL_SHIFT       (4)
#define ADC_PACKET_LEN          (3)         // An ADC read operation requires a 3-byte "packet"

//
// Mapping of GPIO pin names to wiringPi pin numbers for ADC pins
//
typedef enum ADCPin
{
    GPIO_ADC_nCS  = 1       // ADC chip select (active low)
} ADCPin_t;


// ctor - note that we can't use Registry members here; instead we must take explicit args for
// objects which are normally read from the registry.  This is because the ADC is normally init'ed
// from within the Registry ctor, hence we wouldn't be able to obtain a registry instance here.
//
ADC::ADC(GPIOPort& gpio, Config& config, Error * const err)
    : ready_(false)
{
    vref_ = config.get("adc.ref_voltage", ADC_DEFAULT_REF_VOLTAGE);

    // Set ADC_nCS as an output, and de-assert it
    gpio.write(GPIO_ADC_nCS, 1);
    
    if(!gpio.setMode(GPIO_ADC_nCS, PIN_OUTPUT))
    {
        formatError(err, GPIO_PIN_MODE_SET_FAILED);
        return;
    }

    if(vref_ > 0.0)
        ready_ = true;
}


// read() - read a sample from ADC channel <channel>; return the detected voltage through <voltage>.
//
bool ADC::read(const unsigned int channel, double& voltage, Error * const err)
{
    Registry& r = Registry::instance();
    uint8_t tx_data[ADC_PACKET_LEN], rx_data[ADC_PACKET_LEN];
    unsigned short val;

    if(!ready_)
    {
        formatError(err, ADC_NOT_READY);
        return false;
    }

    if(channel > ADC_MAX_CHANNEL)
    {
        formatError(err, ADC_INVALID_CHANNEL);
        return false;
    }

    r.gpio().write(GPIO_ADC_nCS, 0);      // Assert the ADC's nCS line

    tx_data[0] = ADC_START_BIT;
    tx_data[1] = ADC_SINGLE_MODE_BIT | (channel << ADC_CHANNEL_SHIFT);
    tx_data[2] = 0;     // Don't care

    const bool ret = r.spi().transmitAndReceive(tx_data, rx_data, ADC_PACKET_LEN, err);

    r.gpio().write(GPIO_ADC_nCS, 1);      // Negate the ADC's nCS line

    if(ret)
    {
        val = ((rx_data[1] & 0x03) << 8) + rx_data[2];                      // Calculate raw value
        voltage = (double) val * vref_ / (double) ((1 << ADC_BITS) - 1);    // Convert to voltage
    }

    return ret;
}

