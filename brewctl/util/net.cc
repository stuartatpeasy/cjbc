/*
    net.cc: utility functions relating to networking

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/util/net.h"
#include "include/util/random.h"
#include <cstdlib>          // NULL
#include <cstring>          // ::strcpy

extern "C"
{
#include <fcntl.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
}

using std::string;
using std::vector;


namespace Util::Net
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

} // namespace Util::Net

