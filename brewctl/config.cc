/*
    config.cc: provides an abstraction for reading config files

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "config.h"
#include <boost/algorithm/string.hpp>
#include <fstream>

extern "C"
{
#include <syslog.h>
}

using std::istream;
using std::ifstream;
using std::map;
using std::string;
using boost::algorithm::trim;


ConfigData_t Config::data_;


Config::Config()
{
}


Config::~Config()
{
}


void Config::add(istream& is, const char * const name)
{
    string line;
    int linenum;

    for(linenum = 1; is.good() && ! is.eof(); ++linenum)
    {
        getline(is, line);
        trim(line);

        if(line.length())
        {
            if((line[0] == ';') || (line[0] == '#'))
                continue;       // Ignore comments

            size_t delim = line.find('=');
            if((delim == string::npos) || !delim)
            {
                ::syslog(LOG_NOTICE, "config: %s+%d: ignoring line: missing delimiter or key name",
                         name, linenum);
                continue;       // Ignore malformed lines
            }

            string key = line.substr(0, delim), val = line.substr(delim + 1);

            trim(key);
            trim(val);

            data_[key] = val;
        }
    }
}


void Config::add(const char * const filename)
{
    ifstream file(filename);

    add(file, filename);
}


void Config::reset()
{
    data_.clear();
}


bool Config::exists(const char * const key)
{
    return data_.find(key) != data_.end();
}


string Config::operator()(const char * const key)
{
    return exists(key) ? data_[key] : string("");
}


string Config::get(const char * const key, const char * const defaultVal)
{
    return exists(key) ? data_[key] : string(defaultVal);
}

