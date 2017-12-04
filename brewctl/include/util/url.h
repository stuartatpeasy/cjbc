#ifndef UTIL_URL_H_INC
#define UTIL_URL_H_INC
/*
    url.h: provides class URL, which processes a URL string.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include <map>
#include <string>


namespace Util
{

class URL
{
public:
const int NO_PORT_SPECIFIED = -1;
typedef std::map<std::string, std::string> URLQueryArgMap_t;

                        URL() = default;
                        URL(std::string url) noexcept;

    std::string         protocol() const noexcept { return protocol_; };
    std::string         address() const noexcept { return address_; };
    int                 port() const noexcept { return port_; };
    std::string         path() const noexcept { return path_; };
    std::string         anchor() const noexcept { return anchor_; };

    URLQueryArgMap_t&   args() noexcept { return args_; };
    bool                argExists(const std::string& key) const noexcept { return args_.find(key) != args_.end(); };

    URL&                parseURL(std::string url) noexcept;

    static std::string  decode(const std::string& str) noexcept;

private:
    std::string         protocol_;
    std::string         address_;
    int                 port_;
    std::string         path_;
    std::string         anchor_;
    URLQueryArgMap_t    args_;
};

} // namespace Util

#endif // UTIL_URL_H_INC

