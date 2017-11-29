/*
    util.cc: utility functions

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "util.h"
#include <chrono>
#include <cstdlib>      // NULL
#include <cstring>      // ::strcpy
#include <random>

extern "C"
{
#include <netdb.h>
#include <net/if.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

using std::chrono::duration;
using std::chrono::high_resolution_clock;
using std::string;
using std::vector;


namespace Util
{
namespace Net
{

const unsigned short HWADDR_LEN = 6;        // Length, in bytes, of a MAC address


// getInterfaceNames() - return the names of all network interfaces in the system, in the form of a vector<string>.
//
vector<string> getInterfaceNames() noexcept
{
    struct if_nameindex *if_nidxs, *interface;
    vector<string> ret;

    if_nidxs = ::if_nameindex();
    if(if_nidxs != NULL)
    {
        for(interface = if_nidxs; (interface->if_index != 0) || (interface->if_name); ++interface)
            ret.push_back(interface->if_name);
    }

    ::if_freenameindex(if_nidxs);

    return ret;
}

// getInterfaceHardwareAddress() - get the hardware (MAC) address of the specified interface, as a uint64_t.
//
uint64_t getInterfaceHardwareAddress(const string& interface) noexcept
{
    int sock, i;
    struct ifreq ifr;
    uint64_t ret = 0;

    sock = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if(sock < 0)
        return (uint64_t) -1;

    ::strcpy(ifr.ifr_name, interface.c_str());

    if(::ioctl(sock, SIOCGIFHWADDR, &ifr) != 0)
        return (uint64_t) -1;

    for(i = 0; i < HWADDR_LEN; ++i)
        ret = (ret << 8) + (unsigned char) (ifr.ifr_hwaddr.sa_data)[i];

    ::close(sock);

    return ret;
}


// getRandomHardwareAddress() - get a random hardware (MAC) address as a uint64_t.
//
uint64_t getRandomHardwareAddress() noexcept
{
    uint64_t ret = Util::Random::randomUnsignedInt();
    ret = (ret << 32) + Util::Random::randomUnsignedInt();

    return ret & 0xffffffffffff;
}

} // namespace Net

namespace Random
{

static unsigned int seedVal_ = 0;


// seed() - seed the random-number generator with the specified <seedVal>.
//
void seed(unsigned int seedVal) noexcept
{
    seedVal_ = seedVal;
}


// seed() - seed the random-number generator with the current time from the high-resolution clock
//
void seed() noexcept
{
    seed(high_resolution_clock::now().time_since_epoch().count());
}


// randomUnsignedInt() - return a random unsigned integer.
//
unsigned int randomUnsignedInt() noexcept
{
    std::minstd_rand0 generator(seedVal_);
    unsigned int ret = generator();

    seedVal_ = ret;

    return ret;
}

} // namespace Random

namespace Thread
{

// setName() - set the current thread's name/title, as reported by ps, top, etc.  Returns true on success, or false if
// the length of <name> exceeds the limit specified by the operating system.
//
bool setName(const string& name) noexcept
{
    return ::pthread_setname_np(::pthread_self(), name.c_str());
}

} // namespace Thread
} // namespace Util

