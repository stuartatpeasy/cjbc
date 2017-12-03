/*
    jsonobject.cc: wrapper around the libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/jsonobject.h"

using std::string;


// ctor - initialise a new JSON object
//
JsonObject::JsonObject() noexcept
    : type_(OBJECT)
{
    jobj_ = ::json_object_new_object();
}


int JsonObject::length() const noexcept
{
    return ::json_object_object_length(jobj_);
}


string JsonObject::getString() const noexcept
{
    return ::json_object_to_json_string(jobj_);
}


string JsonObject::getStringExt(int flags) const noexcept
{
    return ::json_object_to_json_string_ext(jobj_, flags);
}

