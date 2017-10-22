#ifndef SESSIONMANAGER_H_INC
#define SESSIONMANAGER_H_INC
/*
    sessionmanager.h: manages fermentation / conditioning sessions

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "adc.h"
#include "config.h"
#include "error.h"
#include "gpioport.h"
#include "lcd.h"
#include "spiport.h"
#include "sqlite.h"
#include <cstddef>


class SessionManager
{
public:
                    SessionManager(Config& config, Error * const err = nullptr);
    virtual         ~SessionManager();

    bool            init(Error * const err = nullptr);
    void            run();

private:
    Config&         config_;

    ADC *           adc_;
    GPIOPort        gpio_;
    SPIPort *       spi_;
    SQLite          db_;
    LCD             lcd_;
};

#endif // SESSIONMANAGER_H_INC

