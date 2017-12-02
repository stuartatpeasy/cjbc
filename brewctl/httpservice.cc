/*
    httpservice.cc: manages the web service which supports the brewery controller app.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/httpservice.h"
#include "include/framework/registry.h"
#include "include/framework/thread.h"
#include <cstdlib>          // NULL

extern "C"
{
#include <unistd.h>
}


int handleConnection(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
                     const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) noexcept;


// ctor - member initialisation only
//
HttpService::HttpService(const unsigned short port) noexcept
    : Thread(), port_(port), daemon_(NULL)
{
}


// run() - main exection method for the Service thread.  Start the daemon, accept connections, etc.
//
void HttpService::run() noexcept
{
    running_ = true;
    setName("http");

    while(!stop_)
    {
        daemon_ = ::MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port_, NULL, NULL, ::handleConnection, this);

        ::usleep(1000 * 1000);
    }

    // Stop service
    MHD_stop_daemon(daemon_);
}


// handleConnection() - respond to an inbound connection
//
int HttpService::handleConnection(struct MHD_Connection *connection, const char *url, const char *method,
                                  const char *version, const char *upload_data, size_t *upload_data_size,
                                  void **con_cls) noexcept
{
    (void) connection;
    (void) url;
    (void) method;
    (void) version;
    (void) upload_data;
    (void) upload_data_size;
    (void) con_cls;

    return MHD_NO;
}


// handleConnection(): global callback fn, called when a client connects to the server.  Redirects the call to the
// Server::handleConnection() method.
//
int handleConnection(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
                     const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) noexcept
{
    return ((HttpService *) cls)->handleConnection(connection, url, method, version, upload_data, upload_data_size,
                                                   con_cls);
}

