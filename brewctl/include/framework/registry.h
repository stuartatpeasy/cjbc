#ifndef FRAMEWORK_REGISTRY_H_INC
#define FRAMEWORK_REGISTRY_H_INC
/*
    registry.h: acts as a container for pseudo-global data

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "include/framework/config.h"
#include "include/framework/error.h"
#include "include/peripherals/adc.h"
#include "include/peripherals/buttonmanager.h"
#include "include/peripherals/gpioport.h"
#include "include/peripherals/lcd.h"
#include "include/peripherals/shiftreg.h"
#include "include/peripherals/spiport.h"
#include "include/sqlite/sqlite.h"


class Registry
{
public:
    static bool         init(Config& config, Error * const err) noexcept;

    static Registry&    instance()      noexcept { return *instance_; };

    ADC&                adc()           noexcept { return adc_;             };
    Config&             config()        noexcept { return config_;          };
    SQLite&             db()            noexcept { return db_;              };
    GPIOPort&           gpio()          noexcept { return gpio_;            };
    LCD&                lcd()           noexcept { return lcd_;             };
    SPIPort&            spi()           noexcept { return spi_;             };
    ShiftReg&           sr()            noexcept { return sr_;              };
    ButtonManager&      buttonManager() noexcept { return *buttonManager_;  };

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
    ButtonManager *     buttonManager_;
};

#endif // FRAMEWORK_REGISTRY_H_INC

