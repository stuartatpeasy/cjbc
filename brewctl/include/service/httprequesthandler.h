#ifndef INCLUDE_SERVICE_HTTPREQUESTHANDLER_H_INC
#define INCLUDE_SERVICE_HTTPREQUESTHANDLER_H_INC
/*
    httprequesthandler.cc: handle HTTP requests, i.e. API requests.

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/util/url.h"
#include <map>
#include <string>
#include <vector>


class HttpRequestHandler
{
typedef bool (HttpRequestHandler::*ApiCallHandler_t)(void);
typedef std::map<std::string, ApiCallHandler_t> ApiHandlerTable_t;

typedef enum
{
    HTTP_OK                     = 200,
    HTTP_BAD_REQUEST            = 400,
    HTTP_FORBIDDEN              = 403,
    HTTP_NOT_FOUND              = 401,
    HTTP_INTERNAL_SERVER_ERROR  = 500
} HttpStatus_t;

public:
                                HttpRequestHandler(const std::string& method, const std::string& uri) noexcept;
    virtual                     ~HttpRequestHandler() = default;

    bool                        handleRequest() noexcept;
    HttpStatus_t                statusCode() const noexcept { return statusCode_; };
    size_t                      responseLength() const noexcept { return responseBody_.length(); };
    std::string                 responseBody() const noexcept { return responseBody_; };

private:
    bool                        missingArg(const std::string& arg) noexcept;
    bool                        missingArg(const std::vector<std::string>& arg) noexcept;
    bool                        notFound() noexcept;
    bool                        callOption() noexcept;

    std::string                 method_;
    std::string                 uri_;
    Util::URL                   url_;
    HttpStatus_t                statusCode_;
    std::string                 responseBody_;
    static ApiHandlerTable_t    handlers_;
};

#endif // INCLUDE_SERVICE_HTTPREQUESTHANDLER_H_INC

