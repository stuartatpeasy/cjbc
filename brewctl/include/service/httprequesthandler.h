#ifndef INCLUDE_SERVICE_HTTPREQUESTHANDLER_H_INC
#define INCLUDE_SERVICE_HTTPREQUESTHANDLER_H_INC
/*
    httprequesthandler.cc: handle HTTP requests, i.e. API requests.

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include <string>


class HttpRequestHandler
{
public:
                    HttpRequestHandler(const std::string& method, const std::string& url) noexcept;
    virtual         ~HttpRequestHandler() = default;

    bool            handleRequest() noexcept;
    int             statusCode() const noexcept { return statusCode_; };
    size_t          responseLength() const noexcept { return responseBody_.length(); };
    std::string     responseBody() const noexcept { return responseBody_; };

private:
    std::string     method_;
    std::string     url_;
    int             statusCode_;
    std::string     responseBody_;
};

#endif // INCLUDE_SERVICE_HTTPREQUESTHANDLER_H_INC

