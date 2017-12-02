/*
    main.cc - entry point for brewctl

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "include/application.h"
#include "include/error.h"

extern "C"
{
#include <error.h>          // ::error()
}

/*
                             GPIO pinout
+---------+----+--------+------+------+------+--------+----+---------+
|         |wPi | Pin    |      | Phy  |      |    Pin |wPi |         |
+---------+----+--------+------+------+------+--------+----+---------+
|         |    | 3V3    |      | 1  2 |      |     5V |    |         |
|       - |  8 | SDA1   |   IO | 3  4 |      |     5V |    |         |
|       - |  9 | SCL1   |   IO | 5  6 |      |    GND |    |         |
|  LCD_RS |  7 | GPIO7  |   IO | 7  8 | ALT5 |    TXD | 15 | -       |
|         |    | GND    |      | 9  10| ALT5 |    RXD | 16 | -       |
| SR_RCLK |  0 | GPIO0  |   IO |11  12| IO   |  GPIO1 |  1 | ADC_nCS |
|  SW_BTM |  2 | GPIO2  |   IO |13  14|      |    GND |    |         |
|  SW_TOP |  3 | GPIO3  |   IO |15  16| IO   |  GPIO4 |  4 | ROT_CW  |
|         |    | 3V3    |      |17  18| IO   |  GPIO5 |  5 | ROT_CCW |
|    MOSI | 12 | MOSI   | ALT0 |19  20|      |    GND |    |         |
|    MISO | 13 | MISO   | ALT0 |21  22| IO   |  GPIO6 |  6 | ROT_BTN |
|    SCLK | 14 | SCLK   | ALT0 |23  24| O    |    CE0 | 10 | -       |
|         |    | GND    |      |25  26| O    |    CE1 | 11 | -       |
|       - | 30 | SDA0   |   IO |27  28| IO   |   SCL0 | 31 | -       |
|   LCD_E | 21 | GPIO21 |   IO |29  30| IO   |    GND |    |         |
|  LCD_D0 | 22 | GPIO22 |   IO |31  32| IO   | GPIO26 | 26 | LCD_D4  |
|  LCD_D1 | 23 | GPIO23 |   IO |33  34|      |    GND |    |         |
|  LCD_D2 | 24 | GPIO24 |   IO |35  36| IO   | GPIO27 | 27 | LCD_D5  |
|  LCD_D3 | 25 | GPIO25 |   IO |37  38| IO   | GPIO28 | 28 | LCD_D6  |
|         |    | GND    |      |39  40| IO   | GPIO29 | 29 | LCD_D7  |
+---------+----+--------+------+------+------+--------+----+---------+

*/


// main() - application entry point
//
int main(int argc, char **argv) noexcept
{
    Error err;
    Application app(argc, argv, &err);

    if(err.code())
        ::error(err.code(), 0, err.message().c_str());

    app.run();

    return 0;
}

