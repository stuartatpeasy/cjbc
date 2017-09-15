/*
    shiftreg.cc: SPI driver for two 74xx595 shift registers, connected in series

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "shiftreg.h"
#include <cerrno>

extern "C"
{
#include <unistd.h>
};


//
// Mapping of GPIO pin names to wiringPi pin numbers for shift-register pins
//
typedef enum ShiftRegPin
{
    GPIO_SR_RCLK = 0                // Shift register "register clock" signal
} ShiftRegPin_t;

#define SR_CLOCK_MIN_US     (1)     // Minimum width of the register clock pulse, in microseconds


ShiftReg::ShiftReg(GPIOPort& gpio, SPIPort& spi)
    : Device(), ready_(false), gpio_(gpio), spi_(spi), currentVal_(0)
{
    // Force the register-clock signal to be an output, and de-assert it
    gpio_.write(GPIO_SR_RCLK, 0);

    if(!gpio_.setMode(GPIO_SR_RCLK, PIN_OUTPUT))
        return;

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


bool ShiftReg::write(uint16_t val)
{
    if(!ready_)
    {
        errno_ = EAGAIN;
        return false;
    }

    // Force the register clock low
    gpio_.write(GPIO_SR_RCLK, 0);

    // Transmit the bytes
    for(size_t i = 0; i < sizeof(val); ++i, val >>= 8)
    {
        if(!spi_.transmitByte(val))
        {
            errno_ = spi_.errNo();
            spi_.resetErrNo();

            return false;
        }
    }
        
    errno_ = 0;
    currentVal_ = val;
    strobeRegClk();

    return true;
}

