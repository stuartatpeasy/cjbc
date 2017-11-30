#ifndef UTIL_H_INC
#define UTIL_H_INC
/*
    util.h: utility functions

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "error.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>


namespace Util
{
    bool                        daemonise(Error * const err = nullptr) noexcept;

namespace Net
{
    std::vector<std::string>    getInterfaceNames() noexcept;
    uint64_t                    getInterfaceHardwareAddress(const std::string& interface) noexcept;
    uint64_t                    getRandomHardwareAddress() noexcept;

} // namespace Net

namespace Random
{

    void                        seed(unsigned int seedVal) noexcept;
    void                        seed() noexcept;
    unsigned int                randomUnsignedInt() noexcept;

} // namespace Random

namespace Thread
{
    bool                        setName(const std::string& name) noexcept;
} // namespace Thread

} // namespace Util


#endif // UTIL_H_INC
