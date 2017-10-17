/*
    shiftreg.cc: SPI driver for two 74xx595 shift registers, connected in series

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "shiftreg.h"

extern "C"
{
#include <unistd.h>     // usleep()
}


//
// Mapping of GPIO pin names to wiringPi pin numbers for shift-register pins
//
typedef enum ShiftRegPin
{
    GPIO_SR_RCLK = 0                // Shift register "register clock" signal
} ShiftRegPin_t;

#define SR_CLOCK_MIN_US     (1)     // Minimum width of the register clock pulse, in microseconds
#define SR_LEN_BITS         (16)    // Length of the entire shift register in bits


// ctor - configure GPIO port pins and set the shift register output value to 0.
//
ShiftReg::ShiftReg(GPIOPort& gpio, SPIPort& spi, Error * const err)
    : Device(), ready_(false), gpio_(gpio), spi_(spi), currentVal_(0)
{
    // Force the register-clock signal to be an output, and de-assert it
    gpio_.write(GPIO_SR_RCLK, 0);

    if(!gpio_.setMode(GPIO_SR_RCLK, PIN_OUTPUT))
    {
        formatError(err, GPIO_PIN_MODE_SET_FAILED);
        return;
    }

    // Force all shift-register outputs to zero.
    if(spi_.transmitByte(0) && spi_.transmitByte(0))
    {
        strobeRegClk();
        ready_ = true;
    }
}


ShiftReg::~ShiftReg()
{
}


// strobeRegClk() - strobe the RCLK pin of the 74xx595.  This has the effect of transferring to the
// output pins the last eight bits received in the register.
//
void ShiftReg::strobeRegClk()
{
    gpio_.write(GPIO_SR_RCLK, 1);
    ::usleep(SR_CLOCK_MIN_US);
    gpio_.write(GPIO_SR_RCLK, 0);
}


// write() - write the value in <val> to the shift register.  Returns true on success; on failure,
// returns false.
//
bool ShiftReg::write(const uint16_t val, Error * const err)
{
    uint16_t valLocal = val;

    if(!ready_)
    {
        formatError(err, GPIO_NOT_READY);
        return false;
    }

    // Force the register clock low
    gpio_.write(GPIO_SR_RCLK, 0);

    // Transmit the bytes
    for(size_t i = 0; i < sizeof(val); ++i, valLocal >>= 8)
        if(!spi_.transmitByte(valLocal, err))
            return false;

    currentVal_ = val;
    strobeRegClk();

    return true;
}


// operator|=() - OR the current shift register value with the value in <rhs> and update the shift
// register.  Returns true on success, false otherwise.
//
uint16_t ShiftReg::operator|=(const uint16_t rhs)
{
    write(currentVal_ | rhs);

    // If write() succeeds, currentVal_ will equal the new value (ie. or'ed with rhs), which is the
    // correct return value for the success condition.  If it fails, currentVal_ will be unchanged,
    // which is the appropriate return value for the failure condition.
    return currentVal_;
}


// operator&=() - AND the current shift register value with the value in <rhs> and update the shift
// register.  Returns true on success, false otherwise.
//
uint16_t ShiftReg::operator&=(const uint16_t rhs)
{
    write(currentVal_ & rhs);

    // If write() succeeds, currentVal_ will equal the new value (ie. or'ed with rhs), which is the
    // correct return value for the success condition.  If it fails, currentVal_ will be unchanged,
    // which is the appropriate return value for the failure condition.
    return currentVal_;
}


// operator^=() - exclusive-OR the current shift register value with the value in <rhs> and update
// the shift register.  Returns true on success, false otherwise.
//
uint16_t ShiftReg::operator^=(const uint16_t rhs)
{
    write(currentVal_ ^ rhs);

    // If write() succeeds, currentVal_ will equal the new value (ie. or'ed with rhs), which is the
    // correct return value for the success condition.  If it fails, currentVal_ will be unchanged,
    // which is the appropriate return value for the failure condition.
    return currentVal_;
}


// set() - set bit <bit>.  Returns true on success; false if the set operation failed, or if <bit>
// is out of range.
//
bool ShiftReg::set(const unsigned int bit, Error * const err)
{
    if(bit >= SR_LEN_BITS)
    {
        formatError(err, GPIO_INVALID_PIN);
        return false;
    }

    return write(currentVal_ | (1 << bit), err);
}


// clear() - clear bit <bit>.  Returns true on success; false if the clear operation failed, or if
// <bit> is out of range.
//
bool ShiftReg::clear(const unsigned int bit, Error * const err)
{
    if(bit >= SR_LEN_BITS)
    {
        formatError(err, GPIO_INVALID_PIN);
        return false;
    }

    return write(currentVal_ & ~(1 << bit), err);
}


// toggle() - toggle the value of bit <bit>.  Returns true on success; false if the toggle operation
// failed, or if <bit> is out of range.
//
bool ShiftReg::toggle(const unsigned int bit, Error * const err)
{
    if(bit >= SR_LEN_BITS)
    {
        formatError(err, GPIO_INVALID_PIN);
        return false;
    }

    return write(currentVal_ ^ (1 << bit), err);
}


// isSet() - return true if bit <bit> is set (1); false otherwise.  Returns false if <bit> is out of
// range.
//
bool ShiftReg::isSet(const unsigned int bit, Error * const err)
{
    if(bit >= SR_LEN_BITS)
    {
        formatError(err, GPIO_INVALID_PIN);
        return false;           // Consider out-of-range bits to be cleared
    }

    return currentVal_ & (1 << bit);
}

