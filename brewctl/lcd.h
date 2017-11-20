#ifndef LCD_H_INC
#define LCD_H_INC
/*
    lcd.h: HD44780U 20x4-character LCD driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "gpioport.h"
#include <string>


//
// Character codes for custom LCD symbols
//
typedef enum LCDCustomSym
{
    LCD_CH_ARROW_UP     = 1,
    LCD_CH_ARROW_DOWN   = 2,
    LCD_CH_ARROW_2UP    = 3,
    LCD_CH_ARROW_2DOWN  = 4,
    LCD_CH_ARROW_LEFT   = 5,
    LCD_CH_ARROW_RIGHT  = 6
} LCDCustomSym_t;


class LCD
{
public:
                    LCD(GPIOPort& gpio, Error * const err = nullptr) noexcept;

    void            clear() noexcept;
    int             printAt(const int x, const int y, const std::string& format, ...) noexcept;
    bool            putAt(const int x, const int y, const char c, Error * const err = nullptr) noexcept;
    bool            setCursorPos(const int x, const int , Error * const err = nullptr) noexcept;
    void            init() noexcept;
    bool            backlight(const bool state) noexcept;

protected:
    void            writeCommand(uint8_t cmd) noexcept;
    void            writeData(uint8_t cmd) noexcept;
    void            toggleEClock() noexcept;
};

#endif // LCD_H_INC

