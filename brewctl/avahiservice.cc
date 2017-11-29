/*
    avahiservice.h: publish an Avahi service corresponding to the brewery controller.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "avahiservice.h"
#include "log.h"
#include <cstdio>
#include <cstdlib>

extern "C"
{
#include <unistd.h>     // ::sleep()
}

using std::string;


const unsigned short    AVAHI_SERVICE_DEFAULT_PORT  = 1900;     // Default port on which to announce our service
const int               AVAHI_POLL_INTERVAL_MS      = 250;      // Interval between calls to avahi_simple_poll_iterate()


void avahiClientCallback(AvahiClient *client, AvahiClientState state, void *userdata) noexcept;
void avahiEntryGroupCallback(AvahiEntryGroup *group, AvahiEntryGroupState state, void *userdata) noexcept;


// ctor - allocate a service name; create a new Avahi client
//
AvahiService::AvahiService(const std::string& name, Error * const err) noexcept
    : name_(name), group_(NULL), simplePoll_(NULL), client_(NULL), cname_(nullptr), port_(AVAHI_SERVICE_DEFAULT_PORT)
{
    int error;

    simplePoll_ = ::avahi_simple_poll_new();
    if(!simplePoll_)
    {
        formatError(err, AVAHI_SIMPLE_POLL_CREATE_FAILED);
        return;
    }

    cname_ = ::avahi_strdup(name_.c_str());

    error = 0;
    logDebug("Creating Avahi client in order to publish service '%s'", cname_);
    client_ = ::avahi_client_new(::avahi_simple_poll_get(simplePoll_), (AvahiClientFlags) 0, avahiClientCallback, this,
                                 &error);

    if(!client_)
    {
        formatError(err, AVAHI_CLIENT_CREATE_FAILED);
        ::avahi_simple_poll_free(simplePoll_);
        simplePoll_ = NULL;
        return;
    }
}


// dtor - free resources
//
AvahiService::~AvahiService()
{
    if(client_ != NULL)
        ::avahi_client_free(client_);

    if(simplePoll_ != NULL)
        ::avahi_simple_poll_free(simplePoll_);

    if(cname_ != nullptr)
        ::avahi_free(cname_);
}


// run() - main function.  Runs in its own thread to execute the Avahi simple poll loop.
//
void AvahiService::run() noexcept
{
    if(simplePoll_ != NULL)
    {
        int ret = 0;
        logDebug("AvahiService::run(): entering simple poll loop");
        
        while(!ret)
            ret = ::avahi_simple_poll_iterate(simplePoll_, AVAHI_POLL_INTERVAL_MS);

        if(ret == -1)
            logError("AvahiService::run(): avahi_simple_poll_iterate() failed");
        else if(ret == 1)
            logInfo("AvahiService::run(): quit request has been scheduled");
        else if(!ret)
            logInfo("AvahiService::run(): exiting polling loop");
        else
            logError("AvahiService::run(): unknown return code %d returned by avahi_simple_poll_iterate()", ret);
    }
    else
        logWarning("AvahiService::run(): simplePoll_ is NULL; not entering poll loop");
}


// clientCallback() - receives client events from the Avahi service
//
void AvahiService::clientCallback(AvahiClient *client, AvahiClientState state) noexcept
{
    if(client == NULL)
    {
        logWarning("AvahiService::callback(): client ptr is NULL; ignoring callback");
        return;
    }

    switch(state)
    {
        case AVAHI_CLIENT_S_RUNNING:
            // The server has started successfully, and registered its host name on the network; create our service.
            createService(client);
            break;

        case AVAHI_CLIENT_FAILURE:
            logError("AvahiService::callback(): client failure %s", ::avahi_strerror(::avahi_client_errno(client)));
            break;

        case AVAHI_CLIENT_S_COLLISION:
            // Drop our registered services.  When the server returns to the AVAHI_SERVER_RUNNING state, register the
            // services again with the new host name.
            
            // Fall through

        case AVAHI_CLIENT_S_REGISTERING:
            // Server records are being established, possibly as a consequence of a name change.  Wait for our own
            // records to register until the host name is properly established.
            if(group_)
                ::avahi_entry_group_reset(group_);
            break;

        case AVAHI_CLIENT_CONNECTING:
            break;
    }
}


// entryGroupCallback() - receives entry-group events from the Avahi service.
//
void AvahiService::entryGroupCallback(AvahiEntryGroup *group, AvahiEntryGroupState state) noexcept
{
    if((group != NULL) && (group != group_))
    {
        logWarning("AvahiService::entryGroupCallback(): received callback for an unknown group");
        return;
    }

    switch(state)
    {
        case AVAHI_ENTRY_GROUP_ESTABLISHED:
            logInfo("AvahiService::entryGroupCallback(): service '%s' established successfully", cname_);
            break;

        case AVAHI_ENTRY_GROUP_COLLISION:
            // A service-name collision has occurred.  Choose a new name.
            {
                char *newName = ::avahi_alternative_service_name(cname_);
                logInfo("AvahiService::entryGroupCallback(): service name collision; renaming service '%s' -> '%s'",
                        cname_, newName);
                ::avahi_free(cname_);
                cname_ = newName;
                createService(::avahi_entry_group_get_client(group_));
            }
            break;

        case AVAHI_ENTRY_GROUP_FAILURE:
            logError("AvahiService::entryGroupCallback(): entry group failure: %s",
                     ::avahi_strerror(::avahi_client_errno(::avahi_entry_group_get_client(group_))));
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:      // Sigh...
        case AVAHI_ENTRY_GROUP_REGISTERING:
            break;
    }
}


// createService() - register our service in Avahi.
//
void AvahiService::createService(AvahiClient *client) noexcept
{
    logInfo("AvahiService::createService(): creating service '%s'", name_.c_str());

    if(group_ == NULL)
    {
        group_ = ::avahi_entry_group_new(client, avahiEntryGroupCallback, this);
        if(!group_)
        {
            logError("AvahiService::createService(): failed to create Avahi entry group: %s",
                     ::avahi_strerror(::avahi_client_errno(client)));
            ::avahi_simple_poll_quit(simplePoll_);
            return;
        }
    }

    // If the group is empty (perhaps because it is new, or has been reset), add our entry.
    if(::avahi_entry_group_is_empty(group_))
    {
        logInfo("AvahiService::createService(): adding service '%s'", cname_);

        int ret = ::avahi_entry_group_add_service(group_, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags) 0,
                                                  cname_, "_http._tcp", NULL, NULL, port_, NULL);

        if(ret < 0)
        {
            if(ret == AVAHI_ERR_COLLISION)
            {
                // TODO - deal with name collision
                logError("AvahiService::createService(): name collision for service name '%s'", cname_);
                return;
            }
            else
            {
                logError("AvahiService::createService(): failed to add service '%s': %s", cname_, ::avahi_strerror(ret));
                ::avahi_simple_poll_quit(simplePoll_);
                return;
            }
        }

        ret = ::avahi_entry_group_commit(group_);
        if(ret < 0)
        {
            logError("AvahiService::createService(): failed to commit entry group: %s", ::avahi_strerror(ret));
            ::avahi_simple_poll_quit(simplePoll_);
            return;
        }
    }
}


// avahiClientCallback() - global function which receives client events from the Avahi service and forwards them to the
// member fn in the previously-instantiated AvahiService obj.
//
void avahiClientCallback(AvahiClient *client, AvahiClientState state, void *userdata) noexcept
{
    AvahiService * const svc = (AvahiService *) userdata;
    svc->clientCallback(client, state);
}


// avahiEntryGroupCallback() - global function which receives entry-group events from the Avahi service and forwards
// them to the member fn in the previously-instantiated AvahiService obj.
//
void avahiEntryGroupCallback(AvahiEntryGroup *group, AvahiEntryGroupState state, void *userdata) noexcept
{
    AvahiService * const svc = (AvahiService *) userdata;
    svc->entryGroupCallback(group, state);
}

