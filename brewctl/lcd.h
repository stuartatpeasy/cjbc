#ifndef LCD_H_INC
#define LCD_H_INC
/*
    lcd.h: HD44780U 20x4-character LCD driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "device.h"
#include "gpioport.h"
#include <string>

// Character codes for custom LCD symbols
#define LCD_CH_ARROW_UP     (1)
#define LCD_CH_ARROW_DOWN   (2)
#define LCD_CH_ARROW_2UP    (3)
#define LCD_CH_ARROW_2DOWN  (4)
#define LCD_CH_ARROW_LEFT   (5)
#define LCD_CH_ARROW_RIGHT  (6)


class LCD : public Device
{
public:
                    LCD(GPIOPort& gpio);
    virtual        ~LCD();

    void            clear();
    int             printAt(const int x, const int y, const std::string& format, ...);
    bool            putAt(const int x, const int y, const char c);
    bool            setCursorPos(const int x, const int y);

protected:
    void            init();
    void            writeCommand(uint8_t cmd);
    void            writeData(uint8_t cmd);
    void            toggleEClock();

    GPIOPort        gpio_;
};

#endif // LCD_H_INC

