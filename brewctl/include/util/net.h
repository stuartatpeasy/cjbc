#ifndef UTIL_NET_H_INC
#define UTIL_NET_H_INC
/*
    net.h: utility functions related to networking

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/framework/error.h"
#include <string>
#include <vector>


namespace Util::Net
{
    std::vector<std::string>    getInterfaceNames() noexcept;
    uint64_t                    getInterfaceHardwareAddress(const std::string& interface) noexcept;
    uint64_t                    getRandomHardwareAddress() noexcept;
} // namespace Util::Net

#endif // UTIL_NET_H_INC

