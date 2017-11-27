#ifndef APPLICATION_H_INC
#define APPLICATION_H_INC
/*
    application.h: application container class

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "config.h"
#include "error.h"
#include "sessionmanager.h"
#include <string>
#include <vector>


class Application
{
friend void appSignalHandler(int signum) noexcept;

    // Exit codes
    typedef enum
    {
        E_OK                    = 0,
        E_MISSING_ARGVAL        = 1,        // Missing value for cmdline argument
        E_UNKNOWN_ARG           = 2,        // Unrecognised cmdline switch
        E_FILE_OPEN             = 3,        // Unable to open a required file
        E_MEM                   = 4,        // Unable to allocate memory
        E_DB_OPEN_CREATE        = 5,        // Unable to open or create the database
    } ExitCode_t;

public:
                                Application(int argc, char **argv, Error * const err) noexcept;
    bool                        run() noexcept;

private:
    bool                        parseArgs(int argc, char **argv, Error * const err) noexcept;
    bool                        installQuitHandler(Error * const err) noexcept;
    void                        signalHandler(int signum) noexcept;
    void                        avahiThread() noexcept;

    Config                      config_;
    std::string                 appName_;
//    std::vector<std::string>    configLocations_;

    SessionManager              sessionManager_;
};

#endif // APPLICATION_H_INC

