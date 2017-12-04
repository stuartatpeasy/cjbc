#ifndef SERVICE_HTTPSERVICE_H_INC
#define SERVICE_HTTPSERVICE_H_INC
/*
    httpservice.h: manages the web service which supports the brewery controller app.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/framework/error.h"
#include "include/framework/log.h"
#include "include/framework/thread.h"
#include "include/service/httprequesthandler.h"

extern "C"
{
#include <microhttpd.h>
}


class HttpService : public Thread
{
friend int handleConnection(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
                            const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls)
                            noexcept;
public:
                            HttpService(const unsigned short port) noexcept;
                            HttpService(const HttpService&) = delete;

    HttpService&            operator=(const HttpService&) = delete;

    bool                    run() noexcept;

private:
    int                     handleConnection(struct MHD_Connection *connection, const char *url, const char *method,
                                             const char *version, const char *upload_data, size_t *upload_data_size,
                                             void **con_cls) noexcept;

    const unsigned short    port_;
    MHD_Daemon *            daemon_;
};

#endif // SERVICE_HTTPSERVICE_H_INC

