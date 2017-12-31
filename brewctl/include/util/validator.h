#ifndef UTIL_VALIDATOR_H_INC
#define UTIL_VALIDATOR_H_INC
/*
    validator.h: provides various validation predicates, e.g. for use with the Config::get<T>() method.

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include <string>


namespace Util::Validator
{
    // ge0() - returns true if <val> is greater than or equal to zero
    //
    bool ge0(const short& val)              noexcept;
    bool ge0(const int& val)                noexcept;
    bool ge0(const long& val)               noexcept;
    bool ge0(const long long& val)          noexcept;
    bool ge0(const float& val)              noexcept;
    bool ge0(const double& val)             noexcept;
    bool ge0(const long double& val)        noexcept;

    // gt0() - returns true if <val> is greater than zero
    //
    bool gt0(const short& val)              noexcept;
    bool gt0(const int& val)                noexcept;
    bool gt0(const long& val)               noexcept;
    bool gt0(const long long& val)          noexcept;
    bool gt0(const unsigned short& val)     noexcept;
    bool gt0(const unsigned int& val)       noexcept;
    bool gt0(const unsigned long& val)      noexcept;
    bool gt0(const unsigned long long& val) noexcept;
    bool gt0(const float& val)              noexcept;
    bool gt0(const double& val)             noexcept;
    bool gt0(const long double& val)        noexcept;

    // notEmpty() - returns true if <val> (a string) is non-empty
    //
    bool notEmpty(const std::string& val)   noexcept;

} // namespace Util::Validator

#endif // UTIL_VALIDATOR_H_INC

