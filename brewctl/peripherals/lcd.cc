/*
    lcd.cc: HD44780U 20x4-character LCD driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/peripherals/lcd.h"
#include "include/framework/log.h"
#include "include/framework/registry.h"
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <initializer_list>

using std::lock_guard;
using std::mutex;
using std::string;

extern "C"
{
#include <unistd.h>
}

/*
  LCD module pinout
    +----+------+
    |  1 |  VSS |
    |  2 |  VDD |
    |  3 |  VEE |
    |  4 |   RS |
    |  5 |  R/W |
    |  6 |    E |
    |  7 |  DB0 |
    |  8 |  DB1 |
    |  9 |  DB2 |
    | 10 |  DB3 |
    | 11 |  DB4 |
    | 12 |  DB5 |
    | 13 |  DB6 |
    | 14 |  DB7 |
    | 15 | LEDA |
    | 16 | LEDK |
    +----+------+
*/

// LCD custom characters
static const uint8_t charData[] =
{
    0x00,   // .....
    0x00,   // .....
    0x00,   // .....
    0x00,   // .....    Char 0: blank
    0x00,   // .....
    0x00,   // .....
    0x00,   // .....
    0x00,   // -----
    
    0x04,   // ..1..
    0x04,   // ..1..
    0x0e,   // .111.
    0x0e,   // .111.    Char 1: Single up-arrow
    0x1f,   // 11111
    0x1f,   // 11111
    0x00,   // .....
    0x00,   // -----

    0x00,   // .....
    0x1f,   // 11111
    0x1f,   // 11111
    0x0e,   // .111.    Char 2: Single down-arrow
    0x0e,   // .111.
    0x04,   // ..1..
    0x04,   // ..1..
    0x00,   // -----

    0x04,   // ..1..
    0x0e,   // .111.
    0x1f,   // 11111
    0x00,   // .....    Char 3: Double up-arrow
    0x04,   // ..1..
    0x0e,   // .111.
    0x1f,   // 11111
    0x00,   // -----

    0x1f,   // 11111
    0x0e,   // .111.
    0x04,   // ..1..
    0x00,   // .....    Char 4: Double down-arrow
    0x1f,   // 11111
    0x0e,   // .111.
    0x04,   // ..1..
    0x00,   // -----

    0x02,   // ...1.
    0x06,   // ..11.
    0x0e,   // .111.
    0x1e,   // 1111.    Char 5: Left-arrow
    0x0e,   // .111.
    0x06,   // ..11.
    0x02,   // ...1.
    0x00,   // -----

    0x08,   // .1...
    0x0c,   // .11..
    0x0e,   // .111.
    0x0f,   // .1111    Char 6: Right-arrow
    0x0e,   // .111.
    0x0c,   // .11..
    0x08,   // .1...
    0x00    // -----
};

//
// Mapping of GPIO pin names to wiringPi pin numbers for LCD pins
//
typedef enum LCDPin
{
    GPIO_LCD_RS = 7,    // LCD register select
    GPIO_LCD_E  = 21,   // LCD enable clock
    GPIO_LCD_D0 = 22,   // LCD data bus bit 0
    GPIO_LCD_D1 = 23,   // LCD data bus bit 1
    GPIO_LCD_D2 = 24,   // LCD data bus bit 2
    GPIO_LCD_D3 = 25,   // LCD data bus bit 3
    GPIO_LCD_D4 = 26,   // LCD data bus bit 4
    GPIO_LCD_D5 = 27,   // LCD data bus bit 5
    GPIO_LCD_D6 = 28,   // LCD data bus bit 6
    GPIO_LCD_D7 = 29    // LCD data bus bit 7
} LCDPin_t;


// Display parameters
static const int
    LCD_DISP_WIDTH     = 20,   // Display width in characters
    LCD_DISP_HEIGHT    = 4;    // Display height in characters

//
// LCD controller command codes
//
static const uint8_t
    LCD_CMD_CLEAR              = 0x01,
    LCD_CMD_HOME               = 0x02,
    LCD_CMD_ENTRY_MODE_SET     = 0x04,
    LCD_CMD_DISPLAY_CONTROL    = 0x08,
    LCD_CMD_SHIFT              = 0x10,
    LCD_CMD_FUNCTION_SET       = 0x20,
    LCD_CMD_SET_CGRAM_ADDR     = 0x40,
    LCD_CMD_SET_DDRAM_ADDR     = 0x80;

//
// LCD command args / arg masks
//
static const uint8_t
    LCD_ARG_INCREMENT          = 0x02,
    LCD_ARG_SHIFT              = 0x01,
    LCD_ARG_DISP_ENABLE        = 0x04,
    LCD_ARG_CURSOR_ENABLE      = 0x02,
    LCD_ARG_CURSOR_BLINK       = 0x01,
    LCD_ARG_SHIFT_DISPLAY      = 0x08,
    LCD_ARG_SHIFT_RIGHT        = 0x04,
    LCD_ARG_DATA_LEN           = 0x10,
    LCD_ARG_NUM_LINES          = 0x08,
    LCD_ARG_FONT               = 0x04;

// Address masks
static const uint8_t
    LCD_ARG_CGRAM_ADDR_MASK     = 0x3f,
    LCD_ARG_DDRAM_ADDR_MASK     = 0x7f;

// Minimum width of each state of the "E" clock, in microseconds
static const uint8_t LCD_E_CLK_STATE_TIME_US    = 50;   // 50 microseconds

static const uint8_t LCD_BACKLIGHT_SR_BIT       = 0;    // Shift-register bit which controls the backlight

// LCD initialisation command sequence
static const uint8_t initCommands[] =
{
    LCD_CMD_FUNCTION_SET | LCD_ARG_DATA_LEN | LCD_ARG_NUM_LINES,
    LCD_CMD_DISPLAY_CONTROL,
    LCD_CMD_CLEAR,
    LCD_CMD_ENTRY_MODE_SET | LCD_ARG_INCREMENT,
    LCD_CMD_SET_DDRAM_ADDR,
    LCD_CMD_DISPLAY_CONTROL | LCD_ARG_DISP_ENABLE
};

// ctor - note that we can't use Registry members here; instead we must take explicit args for objects which are
// normally read from the registry.  This is because the LCD is normally init'ed from within the Registry ctor, hence we
// wouldn't be able to obtain a registry instance here.
//
LCD::LCD(GPIOPort& gpio, Error * const err) noexcept
{
    for(auto pin_id : {GPIO_LCD_RS, GPIO_LCD_E, GPIO_LCD_D0, GPIO_LCD_D1, GPIO_LCD_D2, GPIO_LCD_D3, GPIO_LCD_D4,
                       GPIO_LCD_D5, GPIO_LCD_D6, GPIO_LCD_D7})
    {
        auto& pin = gpio.pin(pin_id);

        pin.write(false);
        if(!pin.setMode(PIN_OUTPUT, err))
            return;
    }
}


// init() - initialise the LCD by sending the reset command sequence followed by the initialisation sequence.  Returns
// true on success, false otherwise.
//
void LCD::init() noexcept
{
    for(auto i = 0; i < 3; ++i)
    {
        writeCommand(LCD_CMD_FUNCTION_SET | LCD_ARG_DATA_LEN);
        ::usleep(i ? 500 : 10000);
    }

    for(auto cmd: initCommands)
    {
        writeCommand(cmd);
        ::usleep(1000);
    }

    writeCommand(LCD_CMD_HOME);
    ::usleep(2000);

    writeCommand(LCD_CMD_SET_CGRAM_ADDR);

    ::usleep(1000);
    for(size_t i = 0; i < (sizeof(charData) / sizeof(charData[0])); ++i)
    {
        writeData(charData[i]);
        ::usleep(1000);
    }
}


// toggleEClock() - toggle (high -> low) the LCD's "E" (enable) clock.  Returns true on success, false otherwise.
//
void LCD::toggleEClock() noexcept
{
    for(auto i: {1, 0})
    {
        Registry::instance().gpio().pin(GPIO_LCD_E).write(i);
        ::usleep(LCD_E_CLK_STATE_TIME_US);
    }
}


// writeCommand() - write the command <cmd> to the LCD.  Returns true on success, false otherwise.
//
void LCD::writeCommand(uint8_t cmd) noexcept
{
    auto& gpio = Registry::instance().gpio();

    gpio.pin(GPIO_LCD_RS).write(false);     // De-assert RS

    // Place command on data bus
    for(int i = GPIO_LCD_D0; i <= GPIO_LCD_D7; ++i, cmd >>= 1)
        gpio.pin(i).write(cmd & 1);

    toggleEClock();
}


// writeData() - write the data byte <data> to the LCD.  Returns true on success, false otherwise.
//
void LCD::writeData(uint8_t data) noexcept
{
    auto& gpio = Registry::instance().gpio();

    // Set RS=1
    gpio.pin(GPIO_LCD_RS).write(true);

    // Place data on data bus
    for(int i = GPIO_LCD_D0; i <= GPIO_LCD_D7; ++i, data >>= 1)
        gpio.pin(i).write(data & 1);

    toggleEClock();
}


// clear() - erase all content from the LCD screen.  Returns true on success, false otherwise.
//
void LCD::clear() noexcept
{
    writeCommand(LCD_CMD_CLEAR);
}


// setCursorPos() - set the LCD's cursor position to (<x>, <y>).  Returns true on success, false otherwise (including
// cases where <x> or <y> are out of range).
//
bool LCD::setCursorPos(const int x, const int y, Error * const err) noexcept
{
    if((x < 0) || (x >= LCD_DISP_WIDTH) || (y < 0) || (y > LCD_DISP_HEIGHT))
    {
        formatError(err, LCD_INVALID_CURSOR_POS);
        return false;
    }

    uint8_t pos = x;

    if(y & 2)
        pos += 0x14;

    if(y & 1)
        pos += 0x40;

    writeCommand(LCD_CMD_SET_DDRAM_ADDR | pos);
    ::usleep(400);

    return true;
}


// printAt() - printf()-like output method to write text directly to the LCD at position (<x>, <y>).  Returns the number
// of characters actually written, or a negative value on error.
//
int LCD::printAt(const int x, const int y, const string& format, ...) noexcept
{
    va_list ap;
    char buffer[LCD_DISP_WIDTH + 1];

    if(!setCursorPos(x, y))
        return -1;

    va_start(ap, format);

    int ret = vsnprintf(buffer, (sizeof(buffer) / sizeof(buffer[0])) - x, format.c_str(), ap);

    for(auto i = 0; i < ret; ++i)
    {
        writeData(buffer[i]);
        ::usleep(500);
    }

    return ret;
}


// putAt() - putchar()-like method to write a character directly to the LCD at position (<x>, <y>).  Returns true on
// success, false otherwise.
//
bool LCD::putAt(const int x, const int y, const char c, Error * const err) noexcept
{
    if(!setCursorPos(x, y, err))
        return false;

    writeData(c);
    ::usleep(500);

    return true;
}


// backlight() - switch on or off the backlight, according to the value of <state>.  Returns true on success, false
// otherwise.
//
bool LCD::backlight(const bool state) noexcept
{
    auto& sr = Registry::instance().sr();

    return state ? sr.set(LCD_BACKLIGHT_SR_BIT) : sr.clear(LCD_BACKLIGHT_SR_BIT);
}

