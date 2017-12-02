#ifndef SYS_H_INC
#define SYS_H_INC
/*
    sys.h: utility functions related to system operations

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/framework/error.h"
#include <memory>
#include <string>


namespace Util::Sys
{
    bool                        daemonise(const std::string& user, Error * const err = nullptr) noexcept;
} // namespace Util::Sys

#endif // SYS_H_INC

