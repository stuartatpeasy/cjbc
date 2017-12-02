/*
    application.cc: application container class

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/application.h"
#include "include/log.h"
#include "include/registry.h"
#include "include/util/net.h"
#include "include/util/random.h"
#include "include/util/sys.h"
#include "include/util/thread.h"
#include <cerrno>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <queue>
#include <thread>
#include <vector>

extern "C"
{
#include <signal.h>         // ::sigaction()
#include <strings.h>        // ::bzero()
#include <unistd.h>         // ::sleep()
}

using std::ifstream;
using std::ostringstream;
using std::queue;
using std::string;
using std::thread;


void appSignalHandler(int signum) noexcept;
static Application *gApp;

const char * const      DEFAULT_AVAHI_SERVICE_NAME  = "brewctl";
const unsigned short    DEFAULT_HTTP_SERVICE_PORT   = 1900;

// Default values for configuration keys
static ConfigData_t defaultConfig =
{
    {"adc.ref_voltage",             StringValue("5.012")},
    {"adc.isource_ua",              StringValue("146")},                    // ADC current-source current in microamps
    {"application.daemonise",       StringValue("0")},
    {"application.pid_file",        StringValue("/var/run/brewctl.pid")},
    {"application.short_name",      StringValue("brewctl")},
    {"application.user",            StringValue("brewctl")},
    {"database",                    StringValue("brewery.db")},             // FIXME - should be under /var/lib/brewctl
    {"log.method",                  StringValue("syslog")},
    {"log.level",                   StringValue("debug")},
    {"sensor.average_len",          StringValue("1000")},                   // Sensor-reading moving-avg len
    {"sensor.log_interval_s",       StringValue("10")},                     // Interval between sensor readings
    {"service.port",                StringValue("1900")},                   // App web service interface port
    {"session.dead_zone",           StringValue("0.5C")},                   // "Dead zone" for session temp control
    {"session.switch_interval_s",   StringValue("60")},
    {"spi.dev",                     StringValue("/dev/spidev0.0")},
    {"spi.mode",                    StringValue("0")},
    {"spi.max_clock",               StringValue("500000")},
    {"system.avahi_service_name",   StringValue("brewctl")},
};


// ctor - initialise top-level objects; prepare to run application.
//
Application::Application(int argc, char **argv, Error * const err) noexcept
    : avahiService_(nullptr), httpService_(nullptr), systemId_(0)
{
    gApp = this;
    appName_ = argc ? argv[0] : "<NoAppName>";

    Util::Random::seed();

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

    // Daemonise, if specified in config
//    if(config_.strToBool("application.daemonise"))
//        Util::daemonise(config_.get<string>("

    // Register SIGQUIT handler
    if(!installQuitHandler(err))
        return;

    if(!Registry::init(config_, err) ||             // Initialise registry
       !sessionManager_.init(err))                  // Initialise session manager
        return;

    systemId_ = getSystemId();
    ostringstream avahiServiceName;

    avahiServiceName << config_.get<string>("system.avahi_service_name", DEFAULT_AVAHI_SERVICE_NAME)
                     << "-"
                     << std::hex << std::setw(12) << std::setfill('0') << systemId_;

    const unsigned short port = config_.get("service.port", DEFAULT_HTTP_SERVICE_PORT);

    avahiService_ = new AvahiService(avahiServiceName.str(), port, err);
    if(err->code())
        return;

    httpService_ = new HttpService(port);
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
        else if(arg.substr(0, 2) == "-C")
        {
            if(!config_.addLine(arg.substr(2)))
                logWarning("Ignoring malformed command-line config key '%s'", arg.c_str());
        }
        else if(arg == "-D")
            config_.addLine("application.daemonise=1");
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
    thread(&HttpService::run, httpService_).detach();
    thread(&AvahiService::run, avahiService_).detach();
    thread(&SessionManager::run, &sessionManager_).detach();

    Util::Thread::setName(Registry::instance().config()("application.short_name") + ": main");

    while(1)
        ::sleep(1);

    return true;
}


// getSystemId() - get a unique identifier for this system (hardware).  This will be either the hardware (MAC) address
// of the first Ethernet adaptor, or a 48-bit random number.
//
uint64_t Application::getSystemId() noexcept
{
    static uint64_t systemId = 0;

    if(!systemId)
    {
        for(auto interface : Util::Net::getInterfaceNames())
        {
            if((interface.length() >= 3) && (interface.substr(0, 3) == "enx"))
            {
                const uint64_t hwaddr = Util::Net::getInterfaceHardwareAddress(interface);
                if(hwaddr != (uint64_t) -1)
                {
                    logDebug("Application::getSystemId(): Using system ID %012llx (physical address of network "
                             "interface '%s'", hwaddr, interface.c_str());
                    systemId = hwaddr;
                }
                else
                    logDebug("Application::getSystemId(): Failed to get hardware address for interface '%s'",
                             interface.c_str());

                break;
            }
        }

        if(!systemId)
        {
            // No wired Ethernet adaptor found, or unable to retrieve the hardware (MAC) address of the adaptor.
            // Use a random number instead.
            systemId = Util::Net::getRandomHardwareAddress();
            logDebug("Application::getSystemId(): using random number %012llx as system ID", systemId);
        }
    }

    return systemId;
}


// appSignalHandler() - global signal-handler function which forwards signals to the running Application object.
//
void appSignalHandler(int signum) noexcept
{
    gApp->signalHandler(signum);
}

