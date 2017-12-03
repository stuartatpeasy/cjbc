#ifndef FRAMEWORK_APPLICATION_H_INC
#define FRAMEWORK_APPLICATION_H_INC
/*
    application.h: application container class

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/framework/config.h"
#include "include/framework/error.h"
#include "include/service/avahiservice.h"
#include "include/service/httpservice.h"
#include "include/sessionmanager.h"
#include <cstdint>
#include <memory>
#include <string>


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
    bool                        run(Error * const err = nullptr) noexcept;
    void                        stop() noexcept;

private:
    bool                        parseArgs(int argc, char **argv, Error * const err) noexcept;
    bool                        sendQuitSignal(Error * const err = nullptr) noexcept;
    bool                        installQuitHandler(Error * const err) noexcept;
    void                        signalHandler(int signum) noexcept;
    uint64_t                    getSystemId() noexcept;

    Config                      config_;
    std::string                 appName_;
    AvahiService *              avahiService_;
    SessionManager              sessionManager_;
    HttpService *               httpService_;
    uint64_t                    systemId_;
    volatile bool               stop_;
};

#endif // FRAMEWORK_APPLICATION_H_INC

