/*
    shiftreg.cc: SPI driver for two 74xx595 shift registers, connected in series

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/peripherals/shiftreg.h"
#include "include/registry.h"

extern "C"
{
#include <unistd.h>     // usleep()
}

using std::lock_guard;
using std::mutex;

/*
    Shift register bit assignments

    +-----+---------------+
    | bit |      function |
    +-----+---------------+
    |   0 | LCD backlight |
    |   1 |        unused |
    |   2 |        unused |
    |   3 |        unused |
    |   4 |        unused |
    |   5 |        unused |
    |   6 |        unused |
    |   7 |        unused |
    |   8 |    Effector 0 |
    |   9 |    Effector 1 |
    |  10 |    Effector 2 |
    |  11 |    Effector 3 |
    |  12 |    Effector 4 |
    |  13 |    Effector 5 |
    |  14 |    Effector 6 |
    |  15 |    Effector 7 |
    +-----+---------------+
*/

//
// Mapping of GPIO pin names to wiringPi pin numbers for shift-register pins
//
typedef enum ShiftRegPin
{
    GPIO_SR_RCLK = 0                // Shift register "register clock" signal
} ShiftRegPin_t;

static const unsigned int SR_CLOCK_MIN_US   = 1;      // Minimum width of the register clock pulse, in microseconds
static const unsigned int SR_LEN_BITS       = 16;     // Length of the entire shift register in bits


// ctor - configure GPIO port pins and set the shift register output value to 0.
//
ShiftReg::ShiftReg(GPIOPort& gpio, Error * const err) noexcept
    : ready_(false)
{
    // Force the register-clock signal to be an output, and de-assert it
    auto& RClk = gpio.pin(GPIO_SR_RCLK);

    RClk.write(0);
    if(!RClk.setMode(PIN_OUTPUT, err))
        return;
}


// init() - initialise the shift register by forcing all output bits to 0 and setting currentVal_ to 0.  Returns true on
// success; false otherwise.  The shift register is not unusable until this function has been called successfully.
//
bool ShiftReg::init(Error * const err) noexcept
{
    auto& spi = Registry::instance().spi();

    // Force all shift-register outputs to zero.
    if(spi.transmitByte(0, err) && spi.transmitByte(0, err))
    {
        strobeRegClk();
        ready_ = true;
        currentVal_ = 0;
        return true;
    }

    return false;
}


// strobeRegClk() - strobe the RCLK pin of the 74xx595.  This has the effect of transferring to the output pins the last
// eight bits received in the register.
//
void ShiftReg::strobeRegClk() noexcept
{
    auto& RClk = Registry::instance().gpio().pin(GPIO_SR_RCLK);

    RClk.write(true);
    ::usleep(SR_CLOCK_MIN_US);
    RClk.write(false);
}


// write() - write the value in <val> to the shift register.  Returns true on success; on failure, returns false.
//
bool ShiftReg::write(const uint16_t val, Error * const err) noexcept
{
    auto& r = Registry::instance();
    uint16_t valLocal = val;

    if(!ready_)
    {
        formatError(err, GPIO_NOT_READY);
        return false;
    }

    lock_guard<mutex> lock(innerLock_);

    // Force the register clock low
    r.gpio().pin(GPIO_SR_RCLK).write(false);

    // Transmit the bytes
    for(size_t i = 0; i < sizeof(val); ++i, valLocal >>= 8)
        if(!r.spi().transmitByte(valLocal, err))
            return false;

    currentVal_ = val;
    strobeRegClk();

    return true;
}


// operator|=() - OR the current shift register value with the value in <rhs> and update the shift register.  Returns
// true on success, false otherwise.
//
uint16_t ShiftReg::operator|=(const uint16_t rhs) noexcept
{
    lock_guard<mutex> lock(lock_);

    write(currentVal_ | rhs);

    // If write() succeeds, currentVal_ will equal the new value (ie. or'ed with rhs), which is the correct return value
    // for the success condition.  If it fails, currentVal_ will be unchanged, which is the appropriate return value for
    // the failure condition.
    return currentVal_;
}


// operator&=() - AND the current shift register value with the value in <rhs> and update the shift register.  Returns
// true on success, false otherwise.
//
uint16_t ShiftReg::operator&=(const uint16_t rhs) noexcept
{
    lock_guard<mutex> lock(lock_);

    write(currentVal_ & rhs);

    // If write() succeeds, currentVal_ will equal the new value (ie. or'ed with rhs), which is the correct return value
    // for the success condition.  If it fails, currentVal_ will be unchanged, which is the appropriate return value for
    // the failure condition.
    return currentVal_;
}


// operator^=() - exclusive-OR the current shift register value with the value in <rhs> and update the shift register.
// Returns true on success, false otherwise.
//
uint16_t ShiftReg::operator^=(const uint16_t rhs) noexcept
{
    lock_guard<mutex> lock(lock_);

    write(currentVal_ ^ rhs);

    // If write() succeeds, currentVal_ will equal the new value (ie. or'ed with rhs), which is the correct return value
    // for the success condition.  If it fails, currentVal_ will be unchanged, which is the appropriate return value for
    // the failure condition.
    return currentVal_;
}


// set() - set bit <bit>.  Returns true on success; false if the set operation failed, or if <bit> is out of range.
//
bool ShiftReg::set(const unsigned int bit, Error * const err) noexcept
{
    if(bit >= SR_LEN_BITS)
    {
        formatError(err, GPIO_INVALID_PIN);
        return false;
    }

    lock_guard<mutex> lock(lock_);

    return write(currentVal_ | (1 << bit), err);
}


// clear() - clear bit <bit>.  Returns true on success; false if the clear operation failed, or if <bit> is out of
// range.
//
bool ShiftReg::clear(const unsigned int bit, Error * const err) noexcept
{
    if(bit >= SR_LEN_BITS)
    {
        formatError(err, GPIO_INVALID_PIN);
        return false;
    }

    lock_guard<mutex> lock(lock_);

    return write(currentVal_ & ~(1 << bit), err);
}


// toggle() - toggle the value of bit <bit>.  Returns true on success; false if the toggle operation failed, or if <bit>
// is out of range.
//
bool ShiftReg::toggle(const unsigned int bit, Error * const err) noexcept
{
    if(bit >= SR_LEN_BITS)
    {
        formatError(err, GPIO_INVALID_PIN);
        return false;
    }

    lock_guard<mutex> lock(lock_);

    return write(currentVal_ ^ (1 << bit), err);
}


// isSet() - return true if bit <bit> is set (1); false otherwise.  Returns false if <bit> is out of range.
//
bool ShiftReg::isSet(const unsigned int bit, Error * const err) noexcept
{
    if(bit >= SR_LEN_BITS)
    {
        formatError(err, GPIO_INVALID_PIN);
        return false;           // Consider out-of-range bits to be cleared
    }

    lock_guard<mutex> lock(lock_);

    return currentVal_ & (1 << bit);
}

