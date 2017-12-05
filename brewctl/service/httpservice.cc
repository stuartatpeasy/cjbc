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
#include <cstring>          // ::memcpy(), ::strdup()
#include <string>

extern "C"
{
#include <unistd.h>
}

using std::string;


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
        daemon_ = ::MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                                     port_,
                                     NULL,
                                     NULL,
                                     HttpService::callbackHandleConnection, this,
                                     MHD_OPTION_NOTIFY_COMPLETED,
                                     HttpService::callbackRequestCompleted, this,
                                     MHD_OPTION_URI_LOG_CALLBACK,
                                     HttpService::callbackLogUri, this,
                                     MHD_OPTION_END);

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
    (void) version;
    (void) upload_data;
    (void) upload_data_size;
    (void) con_cls;

    struct MHD_Response *response;
    int ret;
    string fullUrl(url);

    if(*con_cls != NULL)
        fullUrl += (const char *) *con_cls;
    
    HttpRequestHandler handler(method, fullUrl);

    handler.handleRequest();    // TODO check return value

    response = MHD_create_response_from_buffer(handler.responseLength(), (void *) handler.responseBody().c_str(),
                                               MHD_RESPMEM_MUST_COPY);

    MHD_add_response_header(response, "Content-Length", Util::String::numberToString(handler.responseLength()).c_str());
    MHD_add_response_header(response, "Content-Type", "text/json");
    MHD_add_response_header(response, "Connection", "close");

    ret = MHD_queue_response(connection, handler.statusCode(), response);
    MHD_destroy_response(response);

    return ret;
}


// logUri() - called via HttpService::callbackLogUri() to receive the full URI of the connection request, including any
// query arguments.
//
void *HttpService::logUri(const char *uri) noexcept
{
    string s(uri);
    s.erase(0, s.find('?'));

    return (void *) ::strdup(s.c_str());
}


// requestCompleted() - called via HttpService::callbackRequestCompleted() to indicate that the current request is
// fully processed.
//
void HttpService::requestCompleted(struct MHD_Connection *connection, void **con_cls,
                                   enum MHD_RequestTerminationCode code) noexcept
{
    (void) connection;
    (void) code;

    if(*con_cls != NULL)
        ::free(*con_cls);    
}


// handleConnection() - static callback fn, called when a client connects to the server.  Redirects the call to the
// HttpServer::handleConnection() method.
//
int HttpService::callbackHandleConnection(void *cls, struct MHD_Connection *connection, const char *url,
                                          const char *method, const char *version, const char *upload_data,
                                          size_t *upload_data_size, void **con_cls) noexcept
{
    return ((HttpService *) cls)->handleConnection(connection, url, method, version, upload_data, upload_data_size,
                                                   con_cls);
}


// callbackLogUri() - static callback fn, called with a request's full URI (including query parameters).  Redirects the
// call to the HttpServer::logUri() method.
//
void *HttpService::callbackLogUri(void *cls, const char *uri) noexcept
{
    return ((HttpService *) cls)->logUri(uri);
}


// callbackRequestCompleted() - static callback fn, called when a request is completed processed.  Redirects the call to
// the HttpServer::requestCompleted() method.
//
void HttpService::callbackRequestCompleted(void *cls, struct MHD_Connection *connection, void **con_cls,
                                           enum MHD_RequestTerminationCode code) noexcept
{
    return ((HttpService *) cls)->requestCompleted(connection, con_cls, code);
}
