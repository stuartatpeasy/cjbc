/*
    objecttype.cc: wrapper around the type-specific libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/objecttype.h"
#include <cstdint>
#include <string>

using std::string;


namespace JSON
{

// ctor<int32_t> - construct an int32_t JSON object with the value specified in <val>
//
template<>
ObjectType<int32_t>::ObjectType(const int32_t& val) noexcept
    : val_(val)
{
    type_ = INT32;
    jobj_ = ::json_object_new_int(val);
}


template<>
int32_t ObjectType<int32_t>::get() const noexcept
{
    return ::json_object_get_int(jobj_);
}


// ctor<int64_t> - construct an int64_t JSON object with the value specified in <val>
//
template<>
ObjectType<int64_t>::ObjectType(const int64_t& val) noexcept
    : val_(val)
{
    type_ = INT64;
    jobj_ = ::json_object_new_int64(val);
}


template<>
int64_t ObjectType<int64_t>::get() const noexcept
{
    return ::json_object_get_int64(jobj_);
}


// ctor<string> - construct a string JSON object with the value specified in <val>
//
template<>
ObjectType<string>::ObjectType(const string& val) noexcept
    : val_(val)
{
    type_ = STRING;
    jobj_ = ::json_object_new_string(val.c_str());
}


template<>
string ObjectType<string>::get() const noexcept
{
    return ::json_object_get_string(jobj_);
}


// ctor<double> - construct a double JSON object with the value specified in <val>
//
template<>
ObjectType<double>::ObjectType(const double& val) noexcept
    : val_(val)
{
    type_ = DOUBLE;
    jobj_ = ::json_object_new_double(val);
}


template<>
double ObjectType<double>::get() const noexcept
{
    return ::json_object_get_double(jobj_);
}


// ctor<bool> - construct a bool JSON object with the value specified in <val>
//
template<>
ObjectType<bool>::ObjectType(const bool& val) noexcept
    : val_(val)
{
    type_ = BOOL;
    jobj_ = ::json_object_new_boolean(val);
}


template<>
bool ObjectType<bool>::get() const noexcept
{
    return ::json_object_get_boolean(jobj_);
}

}

