#ifndef REGISTRY_H_INC
#define REGISTRY_H_INC
/*
    registry.h: acts as a container for pseudo-global data

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


class Registry
{
public:
    static void         init(Config& config, Error * const err);

    static Registry&    instance()  { return *instance_; };

    ADC&                adc()       { return adc_;    };
    Config&             config()    { return config_; };
    SQLite&             db()        { return db_;     };
    GPIOPort&           gpio()      { return gpio_;   };
    LCD&                lcd()       { return lcd_;    };
    SPIPort&            spi()       { return spi_;    };

private:
                        Registry(Config& config, Error * const err);

    static Registry *   instance_;

    SQLite              db_;
    Config&             config_;
    GPIOPort            gpio_;
    SPIPort             spi_;
    ADC                 adc_;
    LCD                 lcd_;
};

#endif // REGISTRY_H_INC

