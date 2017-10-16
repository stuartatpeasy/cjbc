/*
    application.h: application container class

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "application.h"
#include "log.h"
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
    {"adc.ref_voltage",             "5.0"},
    {"database",                    "brewery.db"},      // FIXME - should be under /var/lib/brewctl
    {"log.method",                  "syslog"},
    {"log.level",                   "debug"},
    {"spi.dev",                     "/dev/spidev0.0"},
    {"spi.mode",                    "0"},
    {"spi.max_clock",               "500000"},
    {"thermistor.beta",             "3980"},
    {"thermistor.ref_temp",         "25C"},
    {"thermistor.ref_resistance",   "4700"},
    {"thermistor.isource_ua",       "147"},
};


Application::Application(int argc, char **argv, Error * const err)
{
    appName_ = argc ? argv[0] : "<NoAppName>";

    config_.add(defaultConfig);             // add default values to config
    config_.add("/etc/brewctl.conf");       // default config file location

    if(!parseArgs(argc, argv, err))
        return;

    logInit(config_("log.method"));
    logSetLevel(config_("log.level"));

    if(!db_.open(config_("database"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, err))
    {
        err->format(DB_OPEN_FAILED, config_("database").c_str(),
                    err->message().c_str(), err->code());
        return;
    }

    sessionManager_ = new SessionManager(db_);
    if(sessionManager_ == nullptr)
    {
        err->format(MALLOC_FAILED);
        return;
    }

    spi_ = new SPIPort(gpio_, config_, err);
    if(spi_ == nullptr)
    {
        err->format(MALLOC_FAILED);
        return;
    }

    if(err->code())
        return;

    adc_ = new ADC(&gpio_, spi_, config_);

    sessionManager_->init(err);
    if(err->code())
        return;
}


Application::~Application()
{
    delete sessionManager_;
}


void Application::errExit(const ExitCode_t code, const string& format, ...)
{
    va_list ap;

    va_start(ap, format);

    ::fprintf(stderr, "%s: ", appName_.c_str());
    ::vfprintf(stderr, format.c_str(), ap);
    ::fputc('\n', stderr);

    ::exit(code);
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
