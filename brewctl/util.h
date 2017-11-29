#ifndef UTIL_H_INC
#define UTIL_H_INC
/*
    util.h: utility functions

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include <string>
#include <vector>


namespace Util
{
namespace Net
{
    std::vector<std::string>    getInterfaceNames() noexcept;
    uint64_t                    getInterfaceHardwareAddress(const std::string& interface) noexcept;

} // namespace Net
} // namespace Util


#endif // UTIL_H_INC
