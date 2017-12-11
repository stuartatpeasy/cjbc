#ifndef SERVICE_AVAHISERVICE_H_INC
#define SERVICE_AVAHISERVICE_H_INC
/*
    avahiservice.h: publish an Avahi service corresponding to the brewery controller.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/framework/error.h"
#include "include/framework/thread.h"
#include <memory>
#include <string>

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


class AvahiService : public Thread
{
friend void avahiClientCallback(AvahiClient *client, AvahiClientState state, void *userdata) noexcept;
friend void avahiEntryGroupCallback(AvahiEntryGroup *group, AvahiEntryGroupState state, void *userdata) noexcept;

public:
                            AvahiService(const std::string& name, const unsigned short port,
                                         Error * const err = nullptr) noexcept;
    virtual                 ~AvahiService();

    bool                    run() noexcept override;

private:
    void                    clientCallback(AvahiClient *client, AvahiClientState state) noexcept;
    void                    entryGroupCallback(AvahiEntryGroup *group, AvahiEntryGroupState state) noexcept;
    void                    createService(AvahiClient *client) noexcept;

    std::string             name_;
    AvahiEntryGroup *       group_;
    AvahiSimplePoll *       simplePoll_;
    AvahiClient *           client_;
    char *                  cname_;
    const unsigned short    port_;
};


#endif // SERVICE_AVAHISERVICE_H_INC

