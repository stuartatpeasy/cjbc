/*
    httprequesthandler.cc: handle HTTP requests, i.e. API requests.

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/httprequesthandler.h"
#include "include/framework/log.h"
#include "include/service/json/array.h"
#include "include/service/json/object.h"
#include "include/service/json/element.h"
#include "include/util/url.h"

using std::map;
using std::string;
using std::vector;


// API call-handler table
HttpRequestHandler::ApiHandlerMap_t HttpRequestHandler::handlers_ =
{
    {"/option",         &HttpRequestHandler::callOption}
};


HttpRequestHandler::HttpMethodMap_t HttpRequestHandler::methods_ =
{
    {"HEAD",    HTTP_HEAD},
    {"GET",     HTTP_GET},
    {"POST",    HTTP_POST},
    {"PUT",     HTTP_PUT},
    {"DELETE",  HTTP_DELETE}
};


// ctor - capture request args
//
HttpRequestHandler::HttpRequestHandler(const string& method, const string& uri) noexcept
    : method_(HTTP_INVALID), methodStr_(method), uri_(uri), url_(uri), statusCode_(HTTP_OK)
{
    logDebug("HttpRequestHandler: method=%s uri=%s", method.c_str(), uri.c_str());

    auto m = methods_.find(methodStr_);
    if(m != methods_.end())
        method_ = m->second;
}


// handleRequest() - called by client code when it is ready for us to handle the current request.
//
bool HttpRequestHandler::handleRequest() noexcept
{
    if(method_ == HTTP_INVALID)
        return methodNotAllowed();

    auto handler = handlers_.find(url_.path());
    if(handler == handlers_.end())
        return notFound();

    return (this->*handler->second)();
}


// missingArg() - set this object's response status to HTTP400, and set the response body to a JSON payload indicating
// that the request argument named by <arg> was missing.
//
bool HttpRequestHandler::missingArg(const string& arg) noexcept
{
    statusCode_ = HTTP_BAD_REQUEST;
    responseBody_ = JsonObject().add("missingArg", new JsonElement(arg));

    return true;
}


bool HttpRequestHandler::missingArg(const vector<string>& args) noexcept
{
    if(args.size() == 1)
        return missingArg(args[0]);

    statusCode_ = HTTP_BAD_REQUEST;

    JsonObject o;
    JsonArray a;
//    o.addArray("missingArg"

    return false;       // FIXME
}


// notFound() - set this object's response status to HTTP404, and set the response body to a JSON payload identifying
// the path that could not be found.
//
bool HttpRequestHandler::notFound() noexcept
{
    statusCode_ = HTTP_NOT_FOUND;
    responseBody_ = JsonObject().add("resource", new JsonElement(url_.path()));

    return true;
}


// methodNotAllowed() - set this object's response status to HTTP405, and set the response body to a JSON payload
// identifying the HTTP method that is not permitted for the specified endpoint.
//
bool HttpRequestHandler::methodNotAllowed() noexcept
{
    statusCode_ = HTTP_METHOD_NOT_ALLOWED;
    responseBody_ = JsonObject().add("method", new JsonElement(methodStr_));

    return true;
}

//
// API call handlers follow
//

// callOption() - handle the /option endpoint
//
bool HttpRequestHandler::callOption() noexcept
{
    if(url_.args().find("key") == url_.args().end())
        return missingArg("key");

    return true;
}

