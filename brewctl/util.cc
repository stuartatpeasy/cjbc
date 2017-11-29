/*
    util.h: utility functions

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "util.h"
#include <cstdlib>      // NULL
#include <cstring>      // ::strcpy

extern "C"
{
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

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


// getInterfaceHardwareAddress() - get the hardware (MAC) address of the specified interface, as a hex string.
//
uint64_t getInterfaceHardwareAddress(const string& interface) noexcept
{
    int sock, i;
    struct ifreq ifr;
    uint64_t ret = 0;

    sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == -1)
        return (uint64_t) -1;

    ::strcpy(ifr.ifr_name, interface.c_str());

    if(::ioctl(sock, SIOCGIFHWADDR, &ifr) == -1)
        return (uint64_t) -1;

    for(i = 0; i < HWADDR_LEN; ++i, ret <<= 8)
        ret += ((unsigned char *) ifr.ifr_hwaddr.sa_data)[i];

    ::close(sock);

    return ret;
}

} // namespace Net
} // namespace Util

