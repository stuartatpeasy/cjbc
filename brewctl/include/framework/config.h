#ifndef FRAMEWORK_CONFIG_H_INC
#define FRAMEWORK_CONFIG_H_INC
/*
    config.h: provides an abstraction for reading config files

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/framework/error.h"
#include "include/framework/log.h"
#include "include/stringvalue.h"
#include <map>
#include <memory>
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
    bool                    addLine(std::string line) noexcept;

    void                    reset() noexcept;
    
    bool                    exists(const std::string& key) const noexcept;
    bool                    require(const std::string& key, Error * const err = nullptr) const noexcept;
    std::string             operator()(const std::string& key) noexcept;

    template<typename T> T  get(const std::string& key, const T& defaultVal = T(),
                                bool (*validator)(const T& val) = nullptr) noexcept
                            {
                                if(exists(key))
                                {
                                    T ret = data_[key];
                                    if((validator == nullptr) || validator(ret))
                                        return ret;

                                    logWarning("Config key '%s' has invalid value '%s'; using default value '%s'",
                                                key.c_str(),
                                                StringValue(ret).str().c_str(),
                                                StringValue(defaultVal).str().c_str());
                                }

                                return defaultVal;
                            }

    bool                    strToBool(const std::string& key) noexcept;

    void                    dump(std::ostream& oss) const noexcept;

protected:
    ConfigData_t            data_;
};

#endif // FRAMEWORK_CONFIG_H_INC

