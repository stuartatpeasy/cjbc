#ifndef HTTPSERVICE_H_INC
#define HTTPSERVICE_H_INC
/*
    httpservice.h: manages the web service which supports the brewery controller app.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "error.h"
#include "log.h"

extern "C"
{
#include <microhttpd.h>
}


class HttpService
{
friend int handleConnection(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
                            const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls)
                            noexcept;
public:
                            HttpService(const unsigned short port) noexcept;
                            HttpService(const HttpService&) = delete;

    HttpService&            operator=(const HttpService&) = delete;

    void                    run() noexcept;
    void                    stop() noexcept { stop_ = true; };
    bool                    isRunning() const noexcept { return running_; };

private:
    int                     handleConnection(struct MHD_Connection *connection, const char *url, const char *method,
                                             const char *version, const char *upload_data, size_t *upload_data_size,
                                             void **con_cls) noexcept;

    volatile bool           stop_;
    volatile bool           running_;
    const unsigned short    port_;
    MHD_Daemon *            daemon_;
};


#endif // HTTPSERVICE_H_INC

