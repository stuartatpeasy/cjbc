/*
    application.cc: application container class

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/framework/application.h"
#include "include/framework/log.h"
#include "include/framework/registry.h"
#include "include/util/net.h"
#include "include/util/random.h"
#include "include/util/sys.h"
#include "include/util/thread.h"
#include <cerrno>
#include <cstdlib>
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

static const char * const   DEFAULT_AVAHI_SERVICE_NAME  = "brewctl";
static const char * const   DEFAULT_CONFIG_LOCATION     = "/etc/brewctl.conf";
static const char * const   DEFAULT_APP_NAME            = "<NoAppName>";
static const unsigned short DEFAULT_HTTP_SERVICE_PORT   = 1900;

// Default values for configuration keys
static ConfigData_t defaultConfig =
{
    {"adc.ref_voltage",             StringValue("5.012")},
    {"adc.isource_ua",              StringValue("146")},                    // ADC current-source current in microamps
    {"application.daemonise",       StringValue("0")},
    {"application.pid_file",        StringValue("/home/swallace/cjbc/brewctl/brewctl.pid")},   // FIXME - should be under /var/run/
    {"application.short_name",      StringValue("brewctl")},
    {"application.user",            StringValue("swallace")},
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
    : avahiService_(nullptr), httpService_(nullptr), systemId_(0), stop_(false)
{
    gApp = this;
    appName_ = argc ? argv[0] : DEFAULT_APP_NAME;

    Util::Random::seed();

    config_.add(defaultConfig);             // add default values to config
    config_.add(DEFAULT_CONFIG_LOCATION);   // default config file location

    if(!parseArgs(argc, argv, err))
        return;

    logInit(config_("log.method"));
    logSetLevel(config_("log.level"));
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
    if(signum == SIGQUIT)
    {
        logInfo("Received SIGQUIT; stopping");
        stop();
    }
    else
        logInfo("Ignoring spurious signal %d", signum);
}


// parseArgs() - parse command-line arguments; populate <err> if any errors are found.
//
bool Application::parseArgs(int argc, char **argv, Error * const err) noexcept
{
    queue<string> args;
    for(int i = 1; i < argc; ++i)
        args.push(argv[i]);

    if((args.size() == 1) && (args.back() == "stop"))
    {
        stop_ = true;       // Prevent this instance of the application from starting up
        return sendQuitSignal(err);
    }

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
bool Application::run(Error * const err) noexcept
{
    // Don't start the application if the "stop" flag is set.  This is the case when the application is executed with
    // the "stop" command-line argument - the intention is to stop a running instance of the application, not to start a
    // new one.
    if(stop_)
        return true;

    // Verify that we will be able to drop privileges, if this has been specified in config.  We can drop privileges if
    // we are root, or if the requested runtime user is the same as the current user.
    if(config_.exists("application.user"))
    {
        const string requestedUser = config_.get<string>("application.user");
        const uid_t requestedUid = Util::Sys::getUid(requestedUser);

        if(requestedUid == (uid_t) -1)
            return false;       // No such user

        const uid_t currentUid = ::getuid();

        if(currentUid && (currentUid != requestedUid))
        {
            formatError(err, SWITCH_USER_INSUFFICIENT_PRIV, requestedUser);
            return false;
        }
    }

    // Daemonise, if specified in config
    if(config_.strToBool("application.daemonise") && !Util::Sys::daemonise(err))
        return false;

    // Drop privileges, if specified in config
    if(config_.exists("application.user") && !Util::Sys::setUid(config_.get<string>("application.user"), err))
        return false;

    // Create pidfile
    if(!config_.require("application.pid_file") ||
       !Util::Sys::writePidFile(config_.get<string>("application.pid_file"), err))
        return false;

    if(!Registry::init(config_, err) ||             // Initialise registry
       !sessionManager_.init(err))                  // Initialise session manager
        return false;

    systemId_ = getSystemId();
    ostringstream avahiServiceName;

    avahiServiceName << config_.get<string>("system.avahi_service_name", DEFAULT_AVAHI_SERVICE_NAME)
                     << "-"
                     << std::hex << std::setw(12) << std::setfill('0') << systemId_;

    const unsigned short port = config_.get("service.port", DEFAULT_HTTP_SERVICE_PORT);

    avahiService_ = new AvahiService(avahiServiceName.str(), port, err);
    if(err->code())
        return false;

    httpService_ = new HttpService(port);

    // Register SIGQUIT handler
    if(!installQuitHandler(err))
        return false;

    thread(&HttpService::run, httpService_).detach();
    thread(&AvahiService::run, avahiService_).detach();
    thread(&SessionManager::run, &sessionManager_).detach();

    Util::Thread::setName(Registry::instance().config()("application.short_name") + ": main");

    // Loop until a stop signal is received
    while(!stop_)
        ::sleep(1);

    // Signal child threads to stop
    if(httpService_ != nullptr)
    {
        logInfo("Stopping HTTP service");
        httpService_->stop();
    }

    if(avahiService_ != nullptr)
    {
        logInfo("Stopping Avahi service");
        avahiService_->stop();
    }

    logInfo("Stopping session manager");
    sessionManager_.stop();

    // Wait for child threads to stop
    while(httpService_->isRunning() || avahiService_->isRunning() || sessionManager_.isRunning())
    {
        logInfo("Waiting for child threads to stop");
        ::sleep(1);
    }

    // Remove PID file
    ::unlink(config_.get<string>("application.pid_file").c_str());

    return true;
}


// stop() - stop application
//
void Application::stop() noexcept
{
    stop_ = true;
}


// sendQuitSignal() - look up the PID of a running instance of this application from the application's PID file; send a
// SIGQUIT signal to the process in order to stop it.
//
bool Application::sendQuitSignal(Error * const err) noexcept
{
    if(!config_.require("application.pid_file"))
        return false;

    const int pid = Util::Sys::readPidFile(config_.get<string>("application.pid_file"), err);
    if(pid == -1)
    {
        // If readPidFile() returned -1 and did not set <err>, the PID file doesn't exist, i.e. the application is not
        // running.  In all other cases where -1 is returned, <err> will be set to indicate an error which occurred
        // while attempting to open or read the file.
        if(!err->code())
            formatError(err, NOT_RUNNING);

        return false;
    }

    logInfo("Sending SIGQUIT to process %d", pid);
    errno = 0;
    if(::kill(pid, SIGQUIT) == -1)
    {
        if(errno == ESRCH)
            formatError(err, NOT_RUNNING);
        else
            formatErrorWithErrno(err, SYSCALL_FAILED, "kill()");

        return false;
    }

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

