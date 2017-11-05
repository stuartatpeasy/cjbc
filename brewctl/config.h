#ifndef CONFIG_H_INC
#define CONFIG_H_INC
/*
    config.h: provides an abstraction for reading config files

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include <map>
#include <string>
#include <fstream>


typedef std::map<std::string, std::string> ConfigData_t;


class Config
{
public:
    void                add(std::istream& is, const std::string& name = "<istream>") noexcept;
    void                add(const std::string& filename) noexcept;
    void                add(const ConfigData_t data) noexcept;
    void                add(const std::string& key, const std::string& value) noexcept;

    void                reset() noexcept;
    
    bool                exists(const std::string& key) const noexcept;
    std::string         operator()(const std::string& key) noexcept;
    
    std::string         get(const std::string& key, const std::string& defaultVal = std::string("")) noexcept;
    int                 get(const std::string& key, const int defaultVal = 0) noexcept;
    long                get(const std::string& key, const long defaultVal = 0L) noexcept;
    long long           get(const std::string& key, const long long defaultVal = 0LL) noexcept;
    unsigned long       get(const std::string& key, const unsigned long defaultVal = 0UL) noexcept;
    unsigned long long  get(const std::string& key, const unsigned long long defaultVal = 0ULL) noexcept;
    float               get(const std::string& key, const float defaultVal = 0.0) noexcept;
    double              get(const std::string& key, const double defaultVal = 0.0) noexcept;

    void                dump(std::ostream& oss) const noexcept;

protected:
    ConfigData_t        data_;
};


#endif // CONFIG_H_INC
