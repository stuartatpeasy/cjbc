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
    {"log",                         "syslog"},
    {"spi.dev",                     "/dev/spidev0.0"},
    {"spi.mode",                    "0"},
    {"spi.max_clock",               "500000"},
    {"thermistor.beta",             "3980"},
    {"thermistor.ref_temp",         "25C"},
    {"thermistor.ref_resistance",   "4700"},
    {"thermistor.isource_ua",       "147"},
};

#include <iostream>
Application::Application(int argc, char **argv)
{
    using std::cout;
    using std::endl;

    appName_ = argc ? argv[0] : "<NoAppName>";

    config_.add(defaultConfig);             // add default values to config
    config_.add("/etc/brewctl.conf");       // default config file location

    parseArgs(argc, argv);

    logInit(config_("log").c_str());

    logDebug("hello");

    Error e;
    bool ret = db_.open(config_("database").c_str(),
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, &e);
    if(ret)
    {
        SQLiteStmt stmt;

        ret = db_.prepare("SELECT * FROM session", stmt, &e);
        if(ret)
        {
            stmt.step();
            cout << "Statement returned " << stmt.numCols() << " columns" << endl;
            for(auto i = 0; i < stmt.numCols(); ++i)
            {
                std::cout << i << ": ";
                auto col = stmt.column(i);

                if(col == nullptr)
                   std::cout << "(null)";
                else
                    std::cout << (const char *) *col;
                std::cout << std::endl;
            }
        }
        else
        {
            std::cout << "prepare() error: " << e.message() << std::endl;
        }
    }
    else
    {
        std::cout << "open() error: " << e.message() << std::endl;
    }

    sessionManager_ = new SessionManager(db_);
    sessionManager_->init(&e);
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

