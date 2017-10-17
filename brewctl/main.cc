/*
    main.cc - entry point for brewctl

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "application.h"
#include "error.h"

extern "C"
{
#include <error.h>
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
|       - |  2 | GPIO2  |   IO |13  14|      |    GND |    |         |
|       - |  3 | GPIO3  |   IO |15  16| IO   |  GPIO4 |  4 | -       |
|         |    | 3V3    |      |17  18| IO   |  GPIO5 |  5 | -       |
|    MOSI | 12 | MOSI   | ALT0 |19  20|      |    GND |    |         |
|    MISO | 13 | MISO   | ALT0 |21  22| IO   |  GPIO6 |  6 | -       |
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


int main(int argc, char **argv)
{
    Error err;
    Application app(argc, argv, &err);

    if(err.code())
        error(err.code(), 0, err.message().c_str());

/*
    GPIOPort gpioPort;
    SPIPort spiPort(gpioPort, "/dev/spidev0.0");
    LCD lcd(gpioPort);
    ShiftReg sr(gpioPort, spiPort);

    spiPort.setMode(SPI_MODE_0);        // TODO remove
    spiPort.setBitsPerWord(8);          // TODO remove
    spiPort.setMaxSpeed(500000);        // TODO remove

    ADC adc(gpioPort, spiPort, 5.0);

    Thermistor thermistor(3980, 4700, Temperature(25.0, TEMP_UNIT_CELSIUS));

    TempSensor sensor1(thermistor, adc, 0, 0.000147),
               sensor2(thermistor, adc, 1, 0.000147);

    Temperature T1, T2;

    for(int i = 0; i < 2; ++i)
    {
        lcd.printAt(0, i, "F%d", i + 1);
        lcd.printAt(0, i + 2, "C%d", i + 1);
    }

    lcd.printAt(10, 0, "10d18h");
    lcd.printAt(4, 1, "--.-");
    lcd.printAt(4, 2, "--.-");
    lcd.printAt(4, 3, "--.-");

    sr.set(0);

    for(int i = 0;; ++i)
    {
        sensor1.sense(T1);
        sensor2.sense(T2);

        if(!(i % 100))
        {
            if(T1.C() > -5.0)
                lcd.printAt(4, 0, "%4.1lf\xdf", T1.C() + 0.05);
            else
                lcd.printAt(4, 0, "--.- ");

            if(T2.C() > -5.0)
                lcd.printAt(4, 1, "%4.1lf\xdf", T2.C() + 0.05);
            else
                lcd.printAt(4, 1, "--.- ");

            lcd.putAt(3, 0, LCD_CH_ARROW_2DOWN);

            // XXX output switch 1 = bit 8 .... 8 = bit 15
            sr.toggle(10);
        }

        ::usleep(500 + (::rand() & 0x3ff));
    }
*/
    return 0;
}

