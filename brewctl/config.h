#ifndef CONFIG_H_INC
#define CONFIG_H_INC
/*
    config.h: provides an abstraction for reading config files

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "stringvalue.h"
#include <map>
#include <string>
#include <fstream>


typedef std::map<std::string, StringValue> ConfigData_t;


class Config
{
public:
    void                    add(std::istream& is, const std::string& name = "<istream>") noexcept;
    void                    add(const std::string& filename) noexcept;
    void                    add(const ConfigData_t data) noexcept;
    void                    add(const std::string& key, const std::string& value) noexcept;

    void                    reset() noexcept;
    
    bool                    exists(const std::string& key) const noexcept;
    std::string             operator()(const std::string& key) noexcept;
    
    template<typename T> T  get(const std::string& key, const T& defaultVal = T()) noexcept
                            {
                                return exists(key) ? (T) data_[key] : defaultVal;
                            }

    void                    dump(std::ostream& oss) const noexcept;

protected:
    ConfigData_t            data_;
};


#endif // CONFIG_H_INC
