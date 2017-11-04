/*
    registry.cc: acts as a container for pseudo-global data

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "registry.h"
#include "log.h"
#include <memory>


Registry * Registry::instance_ = nullptr;


// private ctor - initialise the various members in a well-defined order, as there are dependencies.  Attempt to open
// the database file specified in config.
//
Registry::Registry(Config& config, Error * const err)
    : config_(config),
      gpio_(err),
      spi_(gpio_, config_, err),
      sr_(gpio_, err),
      adc_(gpio_, config_, err),
      lcd_(gpio_, err)
{
    if(err->code())
        return;

    // Initialise database object and open database
    if(!db_.open(config_("database"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, err))
    {
        formatError(err, DB_OPEN_FAILED, config_("database").c_str(), err->message().c_str(), err->code());
        return;
    }
}


// init() - functions as a one-off public ctor to initialise the singleton.  Subsequent access to the object is via the
// instance() method.
//
bool Registry::init(Config& config, Error * const err)
{
    if(instance_ == nullptr)
    {
        instance_ = new Registry(config, err);
        if(instance_ == nullptr)
        {
            formatError(err, MALLOC_FAILED);
            return false;
        }

        if(err->code())
            return false;

        // Initialise objects within the registry
        return instance_->sr().init(err) &&
               instance_->lcd().init(err);
    }

    return true;
}

