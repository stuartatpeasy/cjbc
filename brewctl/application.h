#ifndef APPLICATION_H_INC
#define APPLICATION_H_INC
/*
    application.h: application container class

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

// FIXME review these #includes and move as many as possible into application.cc
#include "config.h"
#include "spiport.h"
#include "gpioport.h"
#include "lcd.h"
#include "adc.h"
#include "shiftreg.h"
#include "temperature.h"
#include "thermistor.h"
#include "tempsensor.h"
#include "database.h"
#include <string>
#include <vector>


class Application
{
    // Exit codes
    typedef enum
    {
        E_OK                    = 0,
        E_MISSING_ARGVAL        = 1,
        E_UNKNOWN_ARG           = 2,
        E_FILE_OPEN             = 3
    } ExitCode_t;

public:
                                Application(int argc, char **argv);
    virtual                     ~Application();

protected:
    void                        errExit(const ExitCode_t code, const char * const format, ...);
    void                        parseArgs(int argc, char **argv);

    Config                      config_;
    std::string                 appName_;
//    std::vector<std::string>    configLocations_;

    GPIOPort *                  gpio_;
    SPIPort *                   spi_;
    ADC *                       adc_;
    ShiftReg *                  sr_;
};

#endif // APPLICATION_H_INC

