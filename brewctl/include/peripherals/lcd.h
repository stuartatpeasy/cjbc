#ifndef PERIPHERALS_LCD_H_INC
#define PERIPHERALS_LCD_H_INC
/*
    lcd.h: HD44780U 20x4-character LCD driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/peripherals/gpioport.h"
#include <string>
#include <thread>


//
// Character codes for custom LCD symbols
//
typedef enum LCDCustomSym
{
    LCD_CH_ARROW_UP     = 0,        // Solid arrowhead pointing up
    LCD_CH_ARROW_DOWN   = 1,        //   "       "        "     down
    LCD_CH_ARROW_2UP    = 2,        // Double solid arrowhead pointing up
    LCD_CH_ARROW_2DOWN  = 3,        //   "      "       "        "     down
    LCD_CH_ARROW_LEFT   = 4,        // Solid arrowhead pointing left
    LCD_CH_ARROW_RIGHT  = 5,        //   "       "        "     right
    LCD_CH_WIFI         = 6         // Wi-fi symbol (or a vague approximation thereof)
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
    std::mutex      lock_;
};

#endif // PERIPHERALS_LCD_H_INC

