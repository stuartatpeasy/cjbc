#ifndef SERVICE_H_INC
#define SERVICE_H_INC
/*
    service.h: manages the web service which supports the brewery controller app.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "error.h"
#include "log.h"

extern "C"
{
#include <microhttpd.h>
}


class Service
{
friend int handleConnection(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
                            const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls)
                            noexcept;
public:
                            Service(const unsigned short port) noexcept;
                            Service(const Service&) = delete;

    Service&                operator=(const Service&) = delete;

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


#endif // SERVICE_H_INC

