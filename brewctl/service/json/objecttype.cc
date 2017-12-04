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


// ctor<int64_t> - construct an int64_t JSON object with the value specified in <val>
//
template<>
ObjectType<int64_t>::ObjectType(const int64_t& val) noexcept
    : val_(val)
{
    type_ = INT64;
    jobj_ = ::json_object_new_int64(val);
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


// ctor<double> - construct a double JSON object with the value specified in <val>
//
template<>
ObjectType<double>::ObjectType(const double& val) noexcept
    : val_(val)
{
    type_ = DOUBLE;
    jobj_ = ::json_object_new_double(val);
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


// dtor - free JSON object
//
template<typename T>
ObjectType<T>::~ObjectType() noexcept
{
    ::json_object_put(jobj_);
}


// get<int32_t> - get the value of the JSON object as an int32_t
//
template<>
int32_t ObjectType<int32_t>::get() const noexcept
{
    return ::json_object_get_int(jobj_);
}


// get<int64_t> - get the value of the JSON object as an int64_t
//
template<>
int64_t ObjectType<int64_t>::get() const noexcept
{
    return ::json_object_get_int64(jobj_);
}


// get<string> - get the value of the JSON object as a string
//
template<>
string ObjectType<string>::get() const noexcept
{
    return ::json_object_get_string(jobj_);
}


// get<double> - get the value of the JSON object as a double
//
template<>
double ObjectType<double>::get() const noexcept
{
    return ::json_object_get_double(jobj_);
}


// get<bool> - get the value of the JSON object as a bool
//
template<>
bool ObjectType<bool>::get() const noexcept
{
    return ::json_object_get_boolean(jobj_);
}

} // namespace JSON

