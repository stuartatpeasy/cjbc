/*
    config.cc: provides an abstraction for reading config files

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/

#include "include/framework/config.h"
#include "include/framework/log.h"
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
using boost::algorithm::trim_left;
using boost::algorithm::trim_right;
using boost::iequals;


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
        if(!addLine(line))
            logNotice("config: %s+%d: ignoring line: missing delimiter or key name", name.c_str(), linenum);
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


// addLine() - given a line from a config file (or from a cmd-line argument), in <line>, of the form
// "\s*[:alnum:]+\s*=\s*[:alnum:]*\s*", extract and add a key/value pair and store it in the configuration map.
// If <line> represents a comment (i.e. it matches "^\s*(#|;|//)"), treat it as a comment and ignore it.  If the line is
// empty, ignore it.  Return true in all cases except where a non-empty, non-comment line is not a valid key/value pair.
//
bool Config::addLine(string line) noexcept
{
    trim(line);

    if(!line.length())
        return true;

    // Detect comments
    if((line[0] == '#') || (line[0] == ';') || ((line.length() > 1) && (line[0] == '/') && (line[1] == '/')))
        return true;

    size_t delim = line.find('=');
    if((delim == string::npos) || !delim)
        return false;

    string key = line.substr(0, delim),
           val = line.substr(delim + 1);

    trim_right(key);
    trim_left(val);

    data_[key] = val;

    return true;
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
    return get<string>(key, "");
}


// strToBool() - if the config value identified by <key> equals the string values "yes", "true" or "on", or an integer
// conversion of the value results in a quantity greater than zero, return true.  In all other cases, return false.
// The string tests do not respect case.
//
bool Config::strToBool(const string& key) noexcept
{
    if(exists(key))
    {
        const string& val = data_[key];

        for(auto testVal : {"yes", "true", "on"})
            if(iequals(val, testVal))
                return true;
    }

    return get<int>(key, 0) > 0;
}


// dump() - write all known configuration key/value pairs into the supplied ostream.
//
void Config::dump(ostream& oss) const noexcept
{
    for(auto it : data_)
        oss << it.first << "=" << (string) it.second.str() << endl;
}

