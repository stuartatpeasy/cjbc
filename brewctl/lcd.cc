/*
    lcd.cc: HD44780U 20x4-character LCD driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "lcd.h"
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <initializer_list>

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


#define LCD_DISP_WIDTH          (20)        // Display width in characters
#define LCD_DISP_HEIGHT         (4)         // Display height in characters

//
// LCD controller command codes
//
#define LCD_CMD_CLEAR           (0x01)
#define LCD_CMD_HOME            (0x02)
#define LCD_CMD_ENTRY_MODE_SET  (0x04)
#define LCD_CMD_DISPLAY_CONTROL (0x08)
#define LCD_CMD_SHIFT           (0x10)
#define LCD_CMD_FUNCTION_SET    (0x20)
#define LCD_CMD_SET_CGRAM_ADDR  (0x40)
#define LCD_CMD_SET_DDRAM_ADDR  (0x80)

//
// LCD command args / arg masks
//
#define LCD_ARG_INCREMENT       (0x02)
#define LCD_ARG_SHIFT           (0x01)
#define LCD_ARG_DISP_ENABLE     (0x04)
#define LCD_ARG_CURSOR_ENABLE   (0x02)
#define LCD_ARG_CURSOR_BLINK    (0x01)
#define LCD_ARG_SHIFT_DISPLAY   (0x08)
#define LCD_ARG_SHIFT_RIGHT     (0x04)
#define LCD_ARG_DATA_LEN        (0x10)
#define LCD_ARG_NUM_LINES       (0x08)
#define LCD_ARG_FONT            (0x04)

#define LCD_ARG_CGRAM_ADDR_MASK (0x3f)
#define LCD_ARG_DDRAM_ADDR_MASK (0x7f)

// Minimum width of each state of the "E" clock, in microseconds
#define LCD_E_CLK_STATE_TIME_US (50)        // 50 microseconds


LCD::LCD(GPIOPort& gpio)
    : Device(), gpio_(gpio)
{
    for(auto pin : {GPIO_LCD_RS, GPIO_LCD_E, GPIO_LCD_D0, GPIO_LCD_D1, GPIO_LCD_D2, GPIO_LCD_D3,
                    GPIO_LCD_D4, GPIO_LCD_D5, GPIO_LCD_D6, GPIO_LCD_D7})
    {
        gpio_.write(pin, 0);
        gpio_.setMode(pin, PIN_OUTPUT);
    }

    init();
}


LCD::~LCD()
{
}


void LCD::init()
{
    for(auto i = 0; i < 3; ++i)
    {
        writeCommand(LCD_CMD_FUNCTION_SET | LCD_ARG_DATA_LEN);
        ::usleep(i ? 500 : 10000);
    }

    for(auto cmd : {LCD_CMD_FUNCTION_SET | LCD_ARG_DATA_LEN | LCD_ARG_NUM_LINES,
                    LCD_CMD_DISPLAY_CONTROL,
                    LCD_CMD_CLEAR,
                    LCD_CMD_ENTRY_MODE_SET | LCD_ARG_INCREMENT,
                    LCD_CMD_SET_DDRAM_ADDR,
                    LCD_CMD_DISPLAY_CONTROL | LCD_ARG_DISP_ENABLE})
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


void LCD::toggleEClock()
{
    for(auto i: {1, 0})
    {
        gpio_.write(GPIO_LCD_E, i);
        ::usleep(LCD_E_CLK_STATE_TIME_US);
    }
}


void LCD::writeCommand(uint8_t cmd)
{
    // Set RS=0
    gpio_.write(GPIO_LCD_RS, 0);

    // Place command on data bus
    for(int i = GPIO_LCD_D0; i <= GPIO_LCD_D7; ++i, cmd >>= 1)
        gpio_.write(i, cmd & 1);

    toggleEClock();
}


void LCD::writeData(uint8_t data)
{
    // Set RS=1
    gpio_.write(GPIO_LCD_RS, 1);

    // Place data on data bus
    for(int i = GPIO_LCD_D0; i <= GPIO_LCD_D7; ++i, data >>= 1)
        gpio_.write(i, data & 1);

    toggleEClock();
}


void LCD::clear()
{
    writeCommand(LCD_CMD_CLEAR);
    errno_ = 0;
}


bool LCD::setCursorPos(const int x, const int y)
{
    if((x < 0) || (x >= LCD_DISP_WIDTH) || (y < 0) || (y > LCD_DISP_HEIGHT))
    {
        errno_ = EINVAL;
        return false;
    }

    uint8_t pos = x;

    if(y & 2)
        pos += 0x14;

    if(y & 1)
        pos += 0x40;

    writeCommand(LCD_CMD_SET_DDRAM_ADDR | pos);
    ::usleep(400);
    errno_ = 0;

    return true;
}


int LCD::printAt(const int x, const int y, const string& format, ...)
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

    errno_ = 0;
    return ret;
}


bool LCD::putAt(const int x, const int y, const char c)
{
    if(!setCursorPos(x, y))
        return false;

    writeData(c);
    ::usleep(500);

    return true;
}

