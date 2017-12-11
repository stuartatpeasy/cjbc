/*
    element.cc: wrapper around libjson-c element-specific functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/element.h"
#include "include/framework/log.h"
#include <cstdlib>      // NULL
#include <utility>

using std::string;


// ctor<int32_t> - construct a JSON element with an int32_t value.
//
JsonElement::JsonElement(const int32_t& val) noexcept
{
    jobj_ = ::json_object_new_int(val);
}


// ctor<int64_t> - construct a JSON element with an int64_t value.
//
JsonElement::JsonElement(const int64_t& val) noexcept
{
    jobj_ = ::json_object_new_int64(val);
}


// ctor<bool> - construct a JSON element with a bool value.
//
JsonElement::JsonElement(const bool& val) noexcept
{
    jobj_ = ::json_object_new_boolean(val);
}


// ctor<double> - construct a JSON element with a double value.
//
JsonElement::JsonElement(const double& val) noexcept
{
    jobj_ = ::json_object_new_double(val);
}


// ctor<string> - construct a JSON element with a string value.
//
JsonElement::JsonElement(const string& val) noexcept
{
    jobj_ = ::json_object_new_string(val.c_str());
}


// (private) ctor - construct an object around a pre-existing (and pre-validated) struct json_object.
//
JsonElement::JsonElement(struct json_object *jobj) noexcept
{
    jobj_ = jobj;
}


// get<int32_t> - coerce the value of this JSON element to int32_t, and return it.
//
template<>
int32_t JsonElement::get<int32_t>() noexcept
{
    return ::json_object_get_int(jobj_);
}


// get<int64_t> - coerce the value of this JSON element to int64_t, and return it.
//
template<>
int64_t JsonElement::get<int64_t>() noexcept
{
    return ::json_object_get_int64(jobj_);
}


// get<bool> - coerce the value of this JSON element to bool, and return it.
//
template<>
bool JsonElement::get<bool>() noexcept
{
    return ::json_object_get_boolean(jobj_);
}


// get<double> - coerce the value of this JSON element to double, and return it.
//
template<>
double JsonElement::get<double>() noexcept
{
    return ::json_object_get_double(jobj_);
}


// get<string> - coerce the value of this JSON element to string, and return it.
//
template<>
string JsonElement::get<string>() noexcept
{
    return ::json_object_get_string(jobj_);
}


// static fromPtr() - construct and return a JsonElement from the supplied struct json_object ptr.  If the ptr is NULL,
// or represents a non-scalar JSON type, return NULL.
//
JsonElement * JsonElement::fromPtr(struct json_object *jobj) noexcept
{
    if(jobj == NULL)
    {
        logWarning("JsonElement::fromPtr(): called with NULL ptr");
        return NULL;
    }

    switch(::json_object_get_type(jobj))
    {
        case json_type_boolean:     //
        case json_type_int:         // All these types are acceptable, because they're scalar
        case json_type_double:      //
        case json_type_string:      //
            break;

        case json_type_object:
            logWarning("JsonElement::fromPtr(): argument represents a JSON object; cannot construct");
            return NULL;

        case json_type_array:
            logWarning("JsonElement::fromPtr(): argument represents a JSON array; cannot construct");
            return NULL;

        default:
            logWarning("JsonElement::fromPtr(): argument represents an unknown JSON type; cannot construct");
            return NULL;
    }

    return new JsonElement(jobj);
}

