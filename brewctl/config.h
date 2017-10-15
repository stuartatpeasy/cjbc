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
                        Config();
    virtual             ~Config();

    void                add(std::istream& is, const std::string& name = "<istream>");
    void                add(const std::string& filename);
    void                add(const ConfigData_t data);
    void                add(const std::string& key, const std::string& value);

    void                reset();
    
    bool                exists(const std::string& key) const;
    std::string         operator()(const std::string& key);
    
    std::string         get(const std::string& key,
                            const std::string& defaultVal = std::string(""));
    int                 get(const std::string& key, const int defaultVal = 0);
    long                get(const std::string& key, const long defaultVal = 0L);
    long long           get(const std::string& key, const long long defaultVal = 0LL);
    unsigned long       get(const std::string& key, const unsigned long defaultVal = 0UL);
    unsigned long long  get(const std::string& key, const unsigned long long defaultVal = 0ULL);
    float               get(const std::string& key, const float defaultVal = 0.0);
    double              get(const std::string& key, const double defaultVal = 0.0);

    void                dump(std::ostream& oss) const;

protected:
    ConfigData_t        data_;
};


#endif // CONFIG_H_INC
