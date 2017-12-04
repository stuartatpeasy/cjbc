#ifndef UTIL_STRING_H_INC
#define UTIL_STRING_H_INC
/*
    string.h: utility functions related to strings

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include <memory>
#include <string>


namespace Util::String
{
    bool            isIntStr(const std::string& str, int* intVal = nullptr) noexcept;
    std::string     numberToString(const int& num) noexcept;
} // namespace Util::String

#endif // UTIL_STRING_H_INC

