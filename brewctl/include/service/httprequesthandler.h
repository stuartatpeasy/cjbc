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
public:
    typedef enum
    {
        HTTP_OK                     = 200,
        HTTP_BAD_REQUEST            = 400,
        HTTP_UNAUTHORIZED           = 401,
        HTTP_FORBIDDEN              = 403,
        HTTP_NOT_FOUND              = 404,
        HTTP_METHOD_NOT_ALLOWED     = 405,
        HTTP_NOT_ACCEPTABLE         = 406,
        HTTP_INTERNAL_SERVER_ERROR  = 500
    } HttpStatus_t;

    typedef enum
    {
        HTTP_HEAD,
        HTTP_GET,
        HTTP_POST,
        HTTP_PUT,
        HTTP_DELETE,
        HTTP_INVALID
    } HttpMethod_t;

    typedef bool (HttpRequestHandler::*ApiCallHandler_t)(void);
    typedef std::map<std::string, ApiCallHandler_t> ApiHandlerMap_t;
    typedef std::map<std::string, HttpMethod_t> HttpMethodMap_t;

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
    bool                        methodNotAllowed() noexcept;

    HttpMethod_t                method_;
    std::string                 methodStr_;
    std::string                 uri_;
    Util::URL                   url_;
    HttpStatus_t                statusCode_;
    std::string                 responseBody_;
    static ApiHandlerMap_t      handlers_;
    static HttpMethodMap_t      methods_;
};

#endif // INCLUDE_SERVICE_HTTPREQUESTHANDLER_H_INC

