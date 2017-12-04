/*
    httprequesthandler.cc: handle HTTP requests, i.e. API requests.

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/httprequesthandler.h"

using std::string;


HttpRequestHandler::HttpRequestHandler(const string& method, const string& url) noexcept
    : method_(method), url_(url)
{
}


bool HttpRequestHandler::handleRequest() noexcept
{
    return true;
}

