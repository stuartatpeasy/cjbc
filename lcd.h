#ifndef LCD_H_INC
#define LCD_H_INC
/*
    lcd.h: HD44780U 20x4-character LCD driver

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "device.h"
#include "gpioport.h"


class LCD : public Device
{
public:
                    LCD(GPIOPort& gpio);
    virtual        ~LCD();

    void            clear();
    int             printAt(const int x, const int y, const char * const format, ...);
    bool            setCursorPos(const int x, const int y);

protected:
    void            init();
    void            writeCommand(uint8_t cmd);
    void            writeData(uint8_t cmd);
    void            toggleEClock();

    GPIOPort        gpio_;
};

#endif // LCD_H_INC

