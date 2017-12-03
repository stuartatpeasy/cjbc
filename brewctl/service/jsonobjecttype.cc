/*
    jsonobjecttype.cc: wrapper around the type-specific libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/jsonobjecttype.h"
#include <cstdint>
#include <string>

using std::string;


// ctor<int32_t> - construct an int32_t JSON object with the value specified in <val>
//
template<>
JsonObjectType<int32_t>::JsonObjectType(const int32_t& val) noexcept
    : val_(val)
{
    type_ = INT32;
    jobj_ = ::json_object_new_int(val);
}


template<>
int32_t JsonObjectType<int32_t>::get() const noexcept
{
    return ::json_object_get_int(jobj_);
}


// ctor<int64_t> - construct an int64_t JSON object with the value specified in <val>
//
template<>
JsonObjectType<int64_t>::JsonObjectType(const int64_t& val) noexcept
    : val_(val)
{
    type_ = INT64;
    jobj_ = ::json_object_new_int64(val);
}


template<>
int64_t JsonObjectType<int64_t>::get() const noexcept
{
    return ::json_object_get_int64(jobj_);
}


// ctor<string> - construct a string JSON object with the value specified in <val>
//
template<>
JsonObjectType<string>::JsonObjectType(const string& val) noexcept
    : val_(val)
{
    type_ = STRING;
    jobj_ = ::json_object_new_string(val.c_str());
}


template<>
string JsonObjectType<string>::get() const noexcept
{
    return ::json_object_get_string(jobj_);
}


// ctor<double> - construct a double JSON object with the value specified in <val>
//
template<>
JsonObjectType<double>::JsonObjectType(const double& val) noexcept
    : val_(val)
{
    type_ = DOUBLE;
    jobj_ = ::json_object_new_double(val);
}


template<>
double JsonObjectType<double>::get() const noexcept
{
    return ::json_object_get_double(jobj_);
}


// ctor<bool> - construct a bool JSON object with the value specified in <val>
//
template<>
JsonObjectType<bool>::JsonObjectType(const bool& val) noexcept
    : val_(val)
{
    type_ = BOOL;
    jobj_ = ::json_object_new_boolean(val);
}


template<>
bool JsonObjectType<bool>::get() const noexcept
{
    return ::json_object_get_boolean(jobj_);
}

