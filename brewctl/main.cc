#include "config.h"
#include "spiport.h"
#include "gpioport.h"
#include "lcd.h"
#include "adc.h"
#include "shiftreg.h"
#include "temperature.h"
#include "thermistor.h"
#include "tempsensor.h"
#include <cstdio>
#include <cstdlib>          // rand()

extern "C"
{
#include <syslog.h>
#include <unistd.h>
};

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
    (void) argc;        // Suppress "unused arg" warning
    ::openlog(argv[0], LOG_PID, LOG_DAEMON);

    GPIOPort gpioPort;
    SPIPort spiPort(gpioPort, "/dev/spidev0.0");
    LCD lcd(gpioPort);
    ShiftReg sr(gpioPort, spiPort);
    Config::add("/etc/brewctl.conf");

    spiPort.setMode(SPI_MODE_0);        // TODO remove
    spiPort.setBitsPerWord(8);          // TODO remove
    spiPort.setMaxSpeed(100000);        // TODO remove

    ADC adc(gpioPort, spiPort, 5.0);

    Thermistor thermistor(3980, 4700, Temperature(25.0, TEMP_UNIT_CELSIUS));
    TempSensor sensor(thermistor, adc, 0, 0.0004);

    Temperature T;

    const int ret = lcd.printAt(0, 0, "Hello, world!");
    printf("LCD write: ret=%d\n", ret);

    for(auto i = 0; i <= GPIO_PIN_MAX; ++i)
        printf("pin %d: %s\n", i, gpioPort.read(i) ? "high" : "low");

    for(int i = 0;; ++i)
    {
        sensor.sense(T);

        if(!(i % 100))
            printf("Temperature on channel 0: %.2lfC\n", T.C());

        ::usleep(500 + ::rand() % 1023);
/*
        if(!spiPort.transmitByte(0x55))
        {
            printf("Error: %d\n", spiPort.errNo());
            break;
        }
*/
    }

    return 0;
}

