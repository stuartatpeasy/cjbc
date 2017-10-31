/*
    registry.cc: acts as a container for pseudo-global data

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "registry.h"
#include <memory>


Registry * Registry::instance_ = nullptr;


Registry::Registry(Config& config, Error * const err)
    : config_(config),
      gpio_(err),
      spi_(gpio_, config_, err),
      adc_(gpio_, config_, err),
      lcd_(gpio_, err)
{
    if(err->code())
        return;

    // Initialise database object and open database
    if(!db_.open(config_("database"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, err))
    {
        formatError(err, DB_OPEN_FAILED, config_("database").c_str(), err->message().c_str(),
                    err->code());
        return;
    }
}


bool Registry::init(Config& config, Error * const err)
{
    if(instance_ == nullptr)
    {
        instance_ = new Registry(config, err);
        return !err->code();
    }

    return true;
}

