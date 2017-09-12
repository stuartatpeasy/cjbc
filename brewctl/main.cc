#include "spiport.h"
#include "gpioport.h"
#include "lcd.h"
#include "adc.h"
#include "shiftreg.h"
#include <cstdio>


/*
                   GPIO pinout
+----+--------+------+------+------+--------+----+
|wPi | Pin    |      | Phy  |      |    Pin |wPi |
+----+--------+------+------+------+--------+----+
|    | 3V3    |      | 1  2 |      |     5V |    |
|  8 | SDA1   |   IO | 3  4 |      |     5V |    |
|  9 | SCL1   |   IO | 5  6 |      |    GND |    |
|  7 | GPIO7  |   IO | 7  8 | ALT5 |    TXD | 15 |
|    | GND    |      | 9  10| ALT5 |    RXD | 16 |
|  0 | GPIO0  |   IO |11  12| IO   |  GPIO1 |  1 |
|  2 | GPIO2  |   IO |13  14|      |    GND |    |
|  3 | GPIO3  |   IO |15  16| IO   |  GPIO4 |  4 |
|    | 3V3    |      |17  18| IO   |  GPIO5 |  5 |
| 12 | MOSI   | ALT0 |19  20|      |    GND |    |
| 13 | MISO   | ALT0 |21  22| IO   |  GPIO6 |  6 |
| 14 | SCLK   | ALT0 |23  24| O    |    CE0 | 10 |
|    | GND    |      |25  26| O    |    CE1 | 11 |
| 30 | SDA0   |   IO |27  28| IO   |   SCL0 | 31 |
| 21 | GPIO21 |   IO |29  30| IO   |    GND |    |
| 22 | GPIO22 |   IO |31  32| IO   | GPIO26 | 26 |
| 23 | GPIO23 |   IO |33  34|      |    GND |    |
| 24 | GPIO24 |   IO |35  36| IO   | GPIO27 | 27 |
| 25 | GPIO25 |   IO |37  38| IO   | GPIO28 | 28 |
|    | GND    |      |39  40| IO   | GPIO29 | 29 |
+----+--------+------+------+------+--------+----+

*/


int main(void)
{
    GPIOPort gpioPort;
    SPIPort spiPort(gpioPort, "/dev/spidev0.0");
    LCD lcd(gpioPort);
    ShiftReg sr(gpioPort, spiPort);

    spiPort.setMode(SPI_MODE_0);        // TODO remove
    spiPort.setBitsPerWord(8);          // TODO remove
    spiPort.setMaxSpeed(500000);      // TODO remove

    ADC adc(gpioPort, spiPort, 5.0);

    for(auto i = 0; i <= GPIO_PIN_MAX; ++i)
        printf("pin %d: %s\n", i, gpioPort.read(i) ? "high" : "low");

    for(;;)
    {
        if(!spiPort.transmitByte(0x55))
        {
            printf("Error: %d\n", spiPort.errNo());
            break;
        }
    }

    return 0;
}

