/*
    string.cc: utility functions relating to strings

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/util/string.h"
#include <cstdlib>      // ::strtol()
#include <limits>

using std::numeric_limits;
using std::stol;
using std::string;


namespace Util::String
{

// isIntStr() - returns bool indicating whether <str> contains an integer represented as a string.  Returns the integer
// through <intVal> if so, and if <intVal> is not equal to nullptr.
//
bool isIntStr(const string& str, int* intVal)
{
    if(str.length())
    {
        char * endptr;
        long longVal = ::strtol(str.c_str(), &endptr, 10);
        if((*endptr == '\0') && (longVal >= numeric_limits<int>::min()) && (longVal <= numeric_limits<int>::max()))
        {
            if(intVal != nullptr)
                *intVal = (int) longVal;
            return true;
        }
    }

    return false;
}

} // namespace Util::String

