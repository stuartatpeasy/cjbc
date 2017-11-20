/*
    config.cc: provides an abstraction for reading config files

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "config.h"
#include "log.h"
#include <boost/algorithm/string.hpp>
#include <fstream>

extern "C"
{
#include <syslog.h>
}

using std::endl;
using std::istream;
using std::ifstream;
using std::map;
using std::ostream;
using std::string;
using boost::algorithm::trim;


// add() - read the supplied input stream <is> (whose "name", e.g. a filename) is given in <name>, and add the key/value
// pairs found in the stream to the configuration store.
//
void Config::add(istream& is, const string& name) noexcept
{
    string line;
    int linenum;

    for(linenum = 1; is.good() && !is.eof(); ++linenum)
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
                logNotice("config: %s+%d: ignoring line: missing delimiter or key name", name.c_str(), linenum);
                continue;       // Ignore malformed lines
            }

            string key = line.substr(0, delim), val = line.substr(delim + 1);

            trim(key);
            trim(val);

            data_[key] = val;
        }
    }
}


// add() - open the file <filename>, and pass it as an input stream to another overload of the add() method in order to
// add the contents of the file to the configuration store.
//
void Config::add(const string& filename) noexcept
{
    ifstream file(filename);

    add(file, filename);
}


// add() - add a collection of values from a ConfigData_t object to the configuration store.
//
void Config::add(const ConfigData_t data) noexcept
{
    for(auto it : data)
        data_[it.first] = it.second;
}


// addItem() - add item named <key> with value <value> to the configuration store
//
void Config::add(const string& key, const string& value) noexcept
{
    data_[key] = value;
}


// reset() - erase all configuration values
//
void Config::reset() noexcept
{
    data_.clear();
}


// exists() - return true if a key named <key> exists; false otherwise.
//
bool Config::exists(const std::string& key) const noexcept
{
    return data_.find(key) != data_.end();
}


// operator() - return the value of the key identified by the argument; return an empty string if no such key exists.
//
string Config::operator()(const string& key) noexcept
{
    return get(key.c_str(), "");
}


// get() - if key <key> exists, return its value as a string; otherwise return <defaultVal>.
//
string Config::get(const string& key, const string& defaultVal) noexcept
{
    return exists(key) ? data_[key] : defaultVal;
}


// get() - if key <key> exists, return its value converted to an int; otherwise return <defaultVal>.
//
int Config::get(const string& key, const int defaultVal) noexcept
{
    return exists(key) ? std::stoi(data_[key]) : defaultVal;
}


// get() - if key <key> exists, return its value converted to an unsigned int; otherwise return <defaultVal>.
//
unsigned int Config::get(const string& key, const unsigned int defaultVal) noexcept
{
    return exists(key) ? std::stoul(data_[key]) : defaultVal;
}


// get() - if key <key> exists, return its value converted to a long; otherwise return <defaultVal>.
//
long Config::get(const string& key, const long defaultVal) noexcept
{
    return exists(key) ? std::stol(data_[key]) : defaultVal;
}


// get() - if key <key> exists, return its value converted to a long long; otherwise return <defaultVal>.
//
long long Config::get(const string& key, const long long defaultVal) noexcept
{
    return exists(key) ? std::stoll(data_[key]) : defaultVal;
}


// get() - if key <key> exists, return its value converted to an unsigned long; otherwise return <defaultVal>.
//
unsigned long Config::get(const string& key, const unsigned long defaultVal) noexcept
{
    return exists(key) ? std::stoul(data_[key]) : defaultVal;
}


// get() - if key <key> exists, return its value converted to an unsigned long long; otherwise return <defaultVal>.
//
unsigned long long Config::get(const string& key, const unsigned long long defaultVal) noexcept
{
    return exists(key) ? std::stoull(data_[key]) : defaultVal;
}


// get() - if key <key> exists, return its value converted to a float; otherwise return <defaultVal>.
//
float Config::get(const string& key, const float defaultVal) noexcept
{
    return exists(key) ? std::stof(data_[key]) : defaultVal;
}


// get() - if key <key> exists, return its value converted to a double; otherwise return <defaultVal>.
//
double Config::get(const string& key, const double defaultVal) noexcept
{
    return exists(key) ? std::stod(data_[key]) : defaultVal;
}


// dump() - write all known configuration key/value pairs into the supplied ostream.
//
void Config::dump(ostream& oss) const noexcept
{
    for(auto it : data_)
        oss << it.first << "=" << it.second << endl;
}

