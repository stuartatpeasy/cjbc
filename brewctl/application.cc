/*
    application.h: application container class

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "application.h"
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
    {"database",                    "/var/lib/brewctl/brewctl.db"},
    {"spi.dev",                     "/dev/spidev0.0"},
    {"spi.mode",                    "0"},
    {"spi.max_clock",               "500000"},
    {"adc.ref_voltage",             "5.0"},
    {"thermistor.beta",             "3980"},
    {"thermistor.ref_temp",         "25C"},
    {"thermistor.ref_resistance",   "4700"},
    {"thermistor.isource_ua",       "155"}
};

#include <iostream>
Application::Application(int argc, char **argv)
{
    appName_ = argc ? argv[0] : "<NoAppName>";

    config_.add(defaultConfig);             // add default values to config
    config_.add("/etc/brewctl.conf");       // default config file location

    parseArgs(argc, argv);

    Temperature t;
    Temperature::fromString(config_("thermistor.ref_temp").c_str(), t);

//    database_.open(config_.get("database"));

    config_.dump(std::cout);
}


Application::~Application()
{
}


void Application::errExit(const ExitCode_t code, const char * const format, ...)
{
    va_list ap;

    va_start(ap, format);

    ::fprintf(stderr, "%s: ", appName_.c_str());
    ::vfprintf(stderr, format, ap);
    ::fputc('\n', stderr);

    ::exit(code);
}


void Application::parseArgs(int argc, char **argv)
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
                errExit(E_MISSING_ARGVAL, "Missing value for argument '%s'", arg.c_str());

            const string fileName = args.front();
            args.pop();

            ifstream cfgFile(fileName);
            if(!cfgFile.good())
                errExit(E_FILE_OPEN, "Cannot open config file '%s'", fileName.c_str());

            config_.add(fileName.c_str());
        }
        else
            errExit(E_UNKNOWN_ARG, "Unrecognised argument '%s'", arg.c_str());
    }
}

