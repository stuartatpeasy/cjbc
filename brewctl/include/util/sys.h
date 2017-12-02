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
    bool        daemonise(Error * const err = nullptr) noexcept;
    uid_t       getUid(const std::string& username, Error * const err) noexcept;
} // namespace Util::Sys

#endif // SYS_H_INC

