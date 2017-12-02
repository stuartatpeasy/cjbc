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
#include "shiftreg.h"
#include "spiport.h"
#include "sqlite/sqlite.h"


class Registry
{
public:
    static bool         init(Config& config, Error * const err) noexcept;

    static Registry&    instance()  noexcept { return *instance_; };

    ADC&                adc()       noexcept { return adc_;     };
    Config&             config()    noexcept { return config_;  };
    SQLite&             db()        noexcept { return db_;      };
    GPIOPort&           gpio()      noexcept { return gpio_;    };
    LCD&                lcd()       noexcept { return lcd_;     };
    SPIPort&            spi()       noexcept { return spi_;     };
    ShiftReg&           sr()        noexcept { return sr_;      };

private:
                        Registry(Config& config, Error * const err) noexcept;

    static Registry *   instance_;

    SQLite              db_;
    Config&             config_;
    GPIOPort&           gpio_;
    SPIPort             spi_;
    ShiftReg            sr_;
    ADC                 adc_;
    LCD                 lcd_;
};

#endif // REGISTRY_H_INC

