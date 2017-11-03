/*
    application.h: application container class

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "application.h"
#include "log.h"
#include "registry.h"
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <queue>

using std::ifstream;
using std::string;
using std::queue;


// Default values for configuration keys
static ConfigData_t defaultConfig =
{
    {"adc.ref_voltage",                     "5.0"},
    {"adc.isource_ua",                      "147"},             // ADC current-source current in microamps
    {"database",                            "brewery.db"},      // FIXME - should be under /var/lib/brewctl
    {"log.method",                          "syslog"},
    {"log.level",                           "debug"},
    {"sensor.average_len",                  "1000"},            // Length of moving-average for sensor readings
    {"session.dead_zone",                   "0.5C"},            // "Dead zone" for session temperature control
    {"session.effector_update_interval_s",  "10"},
    {"spi.dev",                             "/dev/spidev0.0"},
    {"spi.mode",                            "0"},
    {"spi.max_clock",                       "500000"},
};


Application::Application(int argc, char **argv, Error * const err)
{
    appName_ = argc ? argv[0] : "<NoAppName>";

    config_.add(defaultConfig);             // add default values to config
    config_.add("/etc/brewctl.conf");       // default config file location

    if(!parseArgs(argc, argv, err))
        return;

    // FIXME error-checking in these methods
    logInit(config_("log.method"));
    logSetLevel(config_("log.level"));

    if(!Registry::init(config_, err) ||             // Initialise registry
       !sessionManager_.init(err))                  // Initialise session manager
        return;

    sessionManager_.run();
}


// parseArgs() - parse command-line arguments; populate <err> if any errors are found.
//
bool Application::parseArgs(int argc, char **argv, Error * const err)
{
    queue<string> args;
    for(int i = 1; i < argc; ++i)
        args.push(argv[i]);

    while(!args.empty())
    {
        const string arg = args.front();
        args.pop();

        if(arg == "-c")
        {
            if(args.empty())
            {
                err->format(MISSING_ARGVAL, arg.c_str());
                return false;
            }

            const string fileName = args.front();
            args.pop();

            ifstream cfgFile(fileName);
            if(!cfgFile.good())
            {
                err->format(CFG_FILE_OPEN_FAILED, fileName.c_str());
                return false;
            }

            config_.add(fileName.c_str());
        }
        else
        {
            err->format(UNKNOWN_ARG, arg.c_str());
            return false;
        }
    }

    return true;
}

