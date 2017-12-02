/*
    adc.cc: MCP3008 8-channel ADC driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/adc.h"
#include "include/registry.h"

using std::lock_guard;
using std::mutex;


static const ADCChannel_t ADC_MAX_CHANNEL   = 7;        // This ADC has channels numbered 0-7
static const unsigned int ADC_BITS          = 10;       // This ADC has 10-bit resolution
static const double ADC_DEFAULT_REF_VOLTAGE = 5.0;      // Default ADC reference voltage
static const double ADC_DEFAULT_ISOURCE_UA  = 147.0;    // Default ADC current-source current, in microamps

static const uint8_t
    ADC_START_BIT           = 1 << 0,   // "Start conversion" flag in ADC command byte
    ADC_SINGLE_MODE_BIT     = 1 << 7,   // Bit representing single-ended mode in ADC command byte
    ADC_CHANNEL_SHIFT       = 4,        // Offset, in bits, of channel number from bit 0 of ADC command byte
    ADC_PACKET_LEN          = 3;        // An ADC read operation requires a 3-byte "packet"

//
// Mapping of GPIO pin names to wiringPi pin numbers for ADC pins
//
typedef enum ADCPin
{
    GPIO_ADC_nCS  = 1       // ADC chip select (active low)
} ADCPin_t;


// ctor - note that we can't use Registry members here; instead we must take explicit args for objects which are
// normally read from the registry.  This is because the ADC is normally init'ed from within the Registry ctor, hence
// we wouldn't be able to obtain a registry instance here.
//
ADC::ADC(GPIOPort& gpio, Config& config, Error * const err) noexcept
    : ready_(false)
{
    vref_ = config.get<double>("adc.ref_voltage", ADC_DEFAULT_REF_VOLTAGE);
    isource_ = config.get<double>("adc.isource_ua", ADC_DEFAULT_ISOURCE_UA) / 1000000.0;    // isource_ is in amps

    // Set ADC_nCS as an output, and de-assert it
    auto& nCS = gpio.pin(GPIO_ADC_nCS);

    nCS.write(true);
    if(!nCS.setMode(PIN_OUTPUT, err))
        return;

    if(vref_ > 0.0)
        ready_ = true;
}


// read() - read a sample from ADC channel <channel>; return the detected voltage or -1.0 in case of error.
//
double ADC::read(const ADCChannel_t channel, Error * const err) noexcept
{
    Registry& r = Registry::instance();
    uint8_t tx_data[ADC_PACKET_LEN], rx_data[ADC_PACKET_LEN];
    unsigned short val;
    auto& nCS = r.gpio().pin(GPIO_ADC_nCS);

    if(!ready_)
    {
        formatError(err, ADC_NOT_READY);
        return -1.0;
    }

    if(channel > ADC_MAX_CHANNEL)
    {
        formatError(err, ADC_INVALID_CHANNEL);
        return -1.0;
    }

    lock_guard<mutex> lock(lock_);

    nCS.write(false);   // Assert the ADC's nCS line

    tx_data[0] = ADC_START_BIT;
    tx_data[1] = ADC_SINGLE_MODE_BIT | (channel << ADC_CHANNEL_SHIFT);
    tx_data[2] = 0;     // Don't care

    const bool ret = r.spi().transmitAndReceive(tx_data, rx_data, ADC_PACKET_LEN, err);

    nCS.write(true);    // Negate the ADC's nCS line
    if(!ret)
        return -1.0;

    val = ((rx_data[1] & 0x03) << 8) + rx_data[2];                      // Calculate raw value
    return (double) val * vref_ / (double) ((1 << ADC_BITS) - 1);       // Convert to voltage and return
}

