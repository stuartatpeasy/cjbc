/*
    application.h: application container class

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "application.h"
#include "log.h"
#include "registry.h"
#include <cerrno>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <queue>
#include <thread>

extern "C"
{
#include <signal.h>         // ::sigaction()
#include <strings.h>        // ::bzero()
#include <unistd.h>         // ::sleep()
}

using std::ifstream;
using std::queue;
using std::string;
using std::thread;

void appSignalHandler(int signum) noexcept;
static Application *gApp;

const char * const DEFAULT_AVAHI_SERVICE_NAME = "brewctl";

// Default values for configuration keys
static ConfigData_t defaultConfig =
{
    {"adc.ref_voltage",                     "5.012"},
    {"adc.isource_ua",                      "146"},                     // ADC current-source current in microamps
    {"application.pid_file",                "/var/run/brewctl.pid"},
    {"application.user",                    "brewctl"},
    {"database",                            "brewery.db"},              // FIXME - should be under /var/lib/brewctl
    {"log.method",                          "syslog"},
    {"log.level",                           "debug"},
    {"sensor.average_len",                  "1000"},                    // Length of moving-average for sensor readings
    {"sensor.log_interval_s",               "10"},                      // Interval between sensor readings
    {"session.dead_zone",                   "0.5C"},                    // "Dead zone" for session temperature control
    {"session.effector_update_interval_s",  "60"},
    {"spi.dev",                             "/dev/spidev0.0"},
    {"spi.mode",                            "0"},
    {"spi.max_clock",                       "500000"},
    {"system.avahi_service_name",           "brewctl"},
};


Application::Application(int argc, char **argv, Error * const err) noexcept
    : avahiService_(nullptr)
{
    gApp = this;
    appName_ = argc ? argv[0] : "<NoAppName>";

    // Check that we're running as root

    // Create pidfile

    // Drop permissions

    config_.add(defaultConfig);             // add default values to config
    config_.add("/etc/brewctl.conf");       // default config file location

    if(!parseArgs(argc, argv, err))
        return;

    // FIXME error-checking in these methods
    logInit(config_("log.method"));
    logSetLevel(config_("log.level"));

    // Register SIGQUIT handler
    if(!installQuitHandler(err))
        return;

    if(!Registry::init(config_, err) ||             // Initialise registry
       !sessionManager_.init(err))                  // Initialise session manager
        return;

    avahiService_ = new AvahiService(config_.get("system.avahi_service_name", DEFAULT_AVAHI_SERVICE_NAME));
}


// installQuitHandler() - install a handler for the SIGQUIT signal.
//
bool Application::installQuitHandler(Error * const err) noexcept
{
    struct sigaction sa;

    ::bzero(&sa, sizeof(sa));
    sa.sa_handler = appSignalHandler;

    if(::sigaction(SIGQUIT, &sa, NULL))
    {
        formatError(err, SIGHANDLER_INSTALL_FAILED, "SIGQUIT", ::strerror(errno), errno);
        return false;
    }

    logDebug("Installed handler for SIGQUIT");
    return true;
}


// signalHandler() - receives and dispatches signal notifications
//
void Application::signalHandler(int signum) noexcept
{
    logDebug("Received signal %d", signum);
}


// parseArgs() - parse command-line arguments; populate <err> if any errors are found.
//
bool Application::parseArgs(int argc, char **argv, Error * const err) noexcept
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


// run() - start application
//
bool Application::run() noexcept
{
    thread(&AvahiService::run, avahiService_).detach();
    thread(&SessionManager::run, &sessionManager_).detach();

    while(1)
        ::sleep(1);

    return true;
}


// appSignalHandler() - global signal-handler function which forwards signals to the running Application object.
//
void appSignalHandler(int signum) noexcept
{
    gApp->signalHandler(signum);
}

