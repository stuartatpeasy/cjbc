/*
    url.cc: provides class URL, which processes a URL string.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/util/url.h"

using std::string;


namespace Util
{

// ctor - parse the supplied URL
//
URL::URL(string url) noexcept
{
    parseURL(url);
}


// parseURL() - parse the supplied URL; store its components in member variables.  Return a ref to the current object.
//
URL& URL::parseURL(string url) noexcept
{
    size_t delim;

    // Extract protocol
    delim = url.find("://");
    if(delim != url.npos)
    {
        protocol_ = url.substr(0, delim);
        url.erase(0, delim + 3);
    }
    else
        protocol_ = "";

    // Extract address and port
    delim = url.find_first_of("/?#");
    string addressAndPort = url.substr(0, delim);
    url.erase(0, delim);

    delim = addressAndPort.find(':');
    address_ = addressAndPort.substr(0, delim);
    addressAndPort.erase(0, delim + 1);

    if(addressAndPort.length())
        port_ = ::strtoul(addressAndPort.c_str(), NULL, 10);
    else
        port_ = NO_PORT_SPECIFIED;

    // Parse path
    delim = url.find('#');
    if(delim != url.npos)
    {
        anchor_ = url.substr(delim + 1);
        url.erase(delim);
    }
    else
        anchor_ = "";

    // Extract query args
    args_.clear();
    delim = url.find('?');
    if(delim != url.npos)
    {
        string query = url.substr(delim + 1);
        url.erase(delim);

        while(query.length())
        {
            delim = query.find('&');

            string var = query.substr(0, delim);
            size_t equals = var.find('=');

            if(equals != var.npos)
                args_[decode(var.substr(0, equals))] = decode(var.substr(equals + 1));
            else
                args_[decode(var.substr(0, equals))] = "";

            query.erase(0, delim);
            query.erase(0, 1);
        }
    }

    path_ = url;

    return *this;
}


// decode() - URL-decode the supplied string <str>; return the URL-decoded version.
//
string URL::decode(const string& str) noexcept
{
    string ret;
    unsigned char val = 0;
    int encoded_seq = 0, bad = 0;

    for(auto it = str.begin(); it != str.end(); ++it)
    {
        if(*it == '%')
            encoded_seq = 1;
        else
        {
            if(!encoded_seq)
                ret += *it;
            else
            {
                if((*it >= '0') && (*it <= '9'))
                    val += *it - '0';
                else if((*it >= 'a') && (*it <= 'f'))
                    val += (*it - 'a') + 10;
                else if((*it >= 'A') && (*it <= 'F'))
                    val += (*it - 'A') + 10;
                else
                    bad = 1;

                if(encoded_seq == 1)
                    val <<= 4;

                if(++encoded_seq == 3)
                {
                    if(!bad)
                        ret += val;
                    val = 0;
                    encoded_seq = 0;
                    bad = 0;
                }
            }
        }
    }

    return ret;
}

} // namespace Util

