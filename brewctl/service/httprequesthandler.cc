/*
    httprequesthandler.cc: handle HTTP requests, i.e. API requests.

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/httprequesthandler.h"
#include "include/service/json/array.h"
#include "include/service/json/object.h"
#include "include/service/json/objecttype.h"
#include "include/util/url.h"

using std::map;
using std::string;
using std::vector;


// API call-handler table
HttpRequestHandler::ApiHandlerTable_t HttpRequestHandler::handlers_ =
{
    {"option",      &HttpRequestHandler::callOption}
};


// ctor - capture request args
//
HttpRequestHandler::HttpRequestHandler(const string& method, const string& uri) noexcept
    : method_(method), uri_(uri), url_(uri), statusCode_(HTTP_OK)
{
}


// handleRequest() - called by client code when it is ready for us to handle the current request.
//
bool HttpRequestHandler::handleRequest() noexcept
{
    auto handler = handlers_.find(url_.path());
    if(handler == handlers_.end())
    {
        // TODO 404
        return true;
    }

    return (this->*handler->second)();
}


// missingArg() - set this object's response status to HTTP400, and set the response body to a JSON payload indicating
// that the request argument named by <arg> was missing.
//
bool HttpRequestHandler::missingArg(const string& arg) noexcept
{
    statusCode_ = HTTP_BAD_REQUEST;
    responseBody_ = JSON::Object().addObject("missingArg", JSON::ObjectType<string>(arg));

    return true;
}


bool HttpRequestHandler::missingArg(const vector<string>& args) noexcept
{
    if(args.count() == 1)
        return missingArg(args[0]);

    statusCode_ = HTTP_BAD_REQUEST;

    JSON::Object o;
    JSON::Array a;
//    o.addArray("missingArg"

    return false;       // FIXME
}


// notFound() - set this object's response status to HTTP404, and set the response body to a JSON payload identifying
// the path that could not be found.
//
bool HttpRequestHandler::notFound() noexcept
{
    statusCode_ = HTTP_NOT_FOUND;
    responseBody_ = JSON::Object().addObject("resource", JSON::ObjectType<string>(url_.path()));

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

