#ifndef AVAHISERVICE_H_INC
#define AVAHISERVICE_H_INC
/*
    avahiservice.h: publish an Avahi service corresponding to the brewery controller.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "error.h"
#include <memory>
#include <string>

// FIXME - check that all of these #includes are needed
extern "C"
{
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/alternative.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>
}


class AvahiService
{
friend void avahiClientCallback(AvahiClient *client, AvahiClientState state, void *userdata) noexcept;
friend void avahiEntryGroupCallback(AvahiEntryGroup *group, AvahiEntryGroupState state, void *userdata) noexcept;

public:
                        AvahiService(const std::string& name, Error * const err = nullptr) noexcept;
    virtual             ~AvahiService();

    void                run() noexcept;

private:
    void                clientCallback(AvahiClient *client, AvahiClientState state) noexcept;
    void                entryGroupCallback(AvahiEntryGroup *group, AvahiEntryGroupState state) noexcept;
    void                createService(AvahiClient *client) noexcept;

    std::string         name_;
    AvahiEntryGroup *   group_;
    AvahiSimplePoll *   simplePoll_;
    AvahiClient *       client_;
    char *              cname_;
    unsigned short      port_;
};


#endif // AVAHISERVICE_H_INC
