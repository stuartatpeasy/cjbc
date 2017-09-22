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

    void                add(std::istream& is, const char * const name = "<istream>");
    void                add(const char * const filename);
    void                add(const ConfigData_t data);
    void                addItem(const char * const key, const char * const value);

    void                reset();
    
    bool                exists(const char * const key);
    std::string         operator()(const char * const key);
    
    std::string         get(const char * const key,
                            const std::string defaultVal = std::string(""));
    int                 get(const char * const key, const int defaultVal = 0);
    long                get(const char * const key, const long defaultVal = 0L);
    long long           get(const char * const key, const long long defaultVal = 0LL);
    unsigned long       get(const char * const key, const unsigned long defaultVal = 0UL);
    unsigned long long  get(const char * const key, const unsigned long long defaultVal = 0ULL);
    float               get(const char * const key, const float defaultVal = 0.0);
    double              get(const char * const key, const double defaultVal = 0.0);

    void                dump(std::ostream& oss) const;

protected:
    ConfigData_t        data_;
};


#endif // CONFIG_H_INC
