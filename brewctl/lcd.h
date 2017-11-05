#ifndef LCD_H_INC
#define LCD_H_INC
/*
    lcd.h: HD44780U 20x4-character LCD driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "gpioport.h"
#include <string>


// Character codes for custom LCD symbols
#define LCD_CH_ARROW_UP     (1)
#define LCD_CH_ARROW_DOWN   (2)
#define LCD_CH_ARROW_2UP    (3)
#define LCD_CH_ARROW_2DOWN  (4)
#define LCD_CH_ARROW_LEFT   (5)
#define LCD_CH_ARROW_RIGHT  (6)


class LCD
{
public:
                    LCD(GPIOPort& gpio, Error * const err = nullptr) noexcept;

    bool            clear(Error * const err = nullptr) noexcept;
    int             printAt(const int x, const int y, const std::string& format, ...) noexcept;
    bool            putAt(const int x, const int y, const char c, Error * const err = nullptr) noexcept;
    bool            setCursorPos(const int x, const int , Error * const err = nullptr) noexcept;
    bool            init(Error * const err = nullptr) noexcept;
    bool            backlight(const bool state) noexcept;

protected:
    bool            writeCommand(uint8_t cmd, Error * const err = nullptr) noexcept;
    bool            writeData(uint8_t cmd, Error * const err = nullptr) noexcept;
    bool            toggleEClock(Error * const err = nullptr) noexcept;
};

#endif // LCD_H_INC

