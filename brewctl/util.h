#ifndef UTIL_H_INC
#define UTIL_H_INC
/*
    util.h: utility functions

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include <cstdint>
#include <string>
#include <vector>


namespace Util
{
namespace Random
{

    void                        seed(unsigned int seedVal) noexcept;
    void                        seed() noexcept;
    unsigned int                randomUnsignedInt() noexcept;

} // namespace Random

namespace Net
{
    std::vector<std::string>    getInterfaceNames() noexcept;
    uint64_t                    getInterfaceHardwareAddress(const std::string& interface) noexcept;
    uint64_t                    getRandomHardwareAddress() noexcept;

} // namespace Net
} // namespace Util


#endif // UTIL_H_INC
