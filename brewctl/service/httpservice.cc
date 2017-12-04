/*
    httpservice.cc: manages the web service which supports the brewery controller app.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/service/httpservice.h"
#include "include/framework/registry.h"
#include "include/framework/thread.h"
#include "include/service/httprequesthandler.h"
#include "include/service/json/objecttype.h"
#include "include/util/string.h"
#include <cstdlib>          // NULL
#include <cstring>          // ::memcpy()
#include <string>

extern "C"
{
#include <unistd.h>
}

using std::string;


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
bool HttpService::run() noexcept
{
    running_ = true;
    setName("http");

    while(!stop_)
    {
        daemon_ = ::MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, port_, NULL, NULL, ::handleConnection, this);

        ::usleep(1000 * 1000);
    }

    // Stop service
    logInfo("HTTP service stopping");
    MHD_stop_daemon(daemon_);
    running_ = false;

    return false;
}


// handleConnection() - respond to an inbound connection
//
int HttpService::handleConnection(struct MHD_Connection *connection, const char *url, const char *method,
                                  const char *version, const char *upload_data, size_t *upload_data_size,
                                  void **con_cls) noexcept
{
    (void) upload_data;
    (void) upload_data_size;
    (void) con_cls;

    struct MHD_Response *response;
    int ret;

    logDebug("HTTP request: method=%s version=%s url=%s", method, version, url);

    HttpRequestHandler handler(method, url);

    handler.handleRequest();    // TODO check return value

    response = MHD_create_response_from_buffer(handler.responseLength(), (void *) handler.responseBody().c_str(),
                                               MHD_RESPMEM_MUST_COPY);

    MHD_add_response_header(response, "Content-Length", Util::String::numberToString(handler.responseLength()).c_str());
    MHD_add_response_header(response, "Content-Type", "text/json");

    ret = MHD_queue_response(connection, handler.statusCode(), response);
    MHD_destroy_response(response);

    return ret;
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

