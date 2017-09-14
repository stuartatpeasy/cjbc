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
protected:
                            Config();
    virtual                 ~Config();

public:
    static void             add(std::istream& is);
    static void             reset();
    static bool             exists(const char * const key);
    std::string             operator()(const char * const key);

protected:
    static ConfigData_t     data_;
};


#endif // CONFIG_H_INC
