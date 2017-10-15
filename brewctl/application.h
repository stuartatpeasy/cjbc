#ifndef APPLICATION_H_INC
#define APPLICATION_H_INC
/*
    application.h: application container class

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "adc.h"
#include "config.h"
#include "gpioport.h"
#include "lcd.h"
#include "sessionmanager.h"
#include "shiftreg.h"
#include "spiport.h"
#include "sqlite.h"
#include "temperature.h"
#include "tempsensor.h"
#include "thermistor.h"
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
    void                        errExit(const ExitCode_t code, const std::string& format, ...);
    void                        parseArgs(int argc, char **argv);

    Config                      config_;
    std::string                 appName_;
//    std::vector<std::string>    configLocations_;

    ADC *                       adc_;
    GPIOPort *                  gpio_;
    SessionManager *            sessionManager_;
    ShiftReg *                  sr_;
    SPIPort *                   spi_;
    SQLite                      db_;
};

#endif // APPLICATION_H_INC

