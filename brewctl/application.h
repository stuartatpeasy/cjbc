#ifndef APPLICATION_H_INC
#define APPLICATION_H_INC
/*
    application.h: application container class

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "adc.h"
#include "config.h"
#include "error.h"
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
        E_MISSING_ARGVAL        = 1,        // Missing value for cmdline argument
        E_UNKNOWN_ARG           = 2,        // Unrecognised cmdline switch
        E_FILE_OPEN             = 3,        // Unable to open a required file
        E_MEM                   = 4,        // Unable to allocate memory
        E_DB_OPEN_CREATE        = 5,        // Unable to open or create the database
    } ExitCode_t;

public:
                                Application(int argc, char **argv, Error * const err);
    virtual                     ~Application();

protected:
    void                        errExit(const ExitCode_t code, const std::string& format, ...);
    bool                        parseArgs(int argc, char **argv, Error * const err);

    Config                      config_;
    std::string                 appName_;
//    std::vector<std::string>    configLocations_;

    ADC *                       adc_;
    GPIOPort                    gpio_;
    SessionManager *            sessionManager_;
    ShiftReg *                  sr_;
    SPIPort *                   spi_;
    SQLite                      db_;
};

#endif // APPLICATION_H_INC

