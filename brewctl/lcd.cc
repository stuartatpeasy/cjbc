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

extern "C"
{
#include <unistd.h>
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


// These LCD pins are outputs
static const LCDPin_t outputs[] =
{
    GPIO_LCD_RS, GPIO_LCD_E, GPIO_LCD_D0, GPIO_LCD_D1, GPIO_LCD_D2, GPIO_LCD_D3, GPIO_LCD_D4,
    GPIO_LCD_D5, GPIO_LCD_D6, GPIO_LCD_D7
};

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
    for(auto pin : outputs)
        gpio.setMode(pin, PIN_OUTPUT);

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
                    LCD_CMD_SET_DDRAM_ADDR})
        writeCommand(cmd);
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


int LCD::printAt(const int x, const int y, const char * const format, ...)
{
    va_list ap;
    char buffer[LCD_DISP_WIDTH];

    if(!setCursorPos(x, y))
        return -1;

    int ret = vsnprintf(buffer, (sizeof(buffer) / sizeof(buffer[0])) - x, format, ap);

    for(auto i = 0; i < ret; ++i)
        writeData(buffer[i]);

    errno_ = 0;
    return ret;
}

