/*
    object.cc: wrapper around the libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/object.h"

using std::string;


namespace JSON
{

// ctor - initialise a new JSON object
//
Object::Object() noexcept
    : type_(OBJECT)
{
    jobj_ = ::json_object_new_object();
}


int Object::length() const noexcept
{
    return ::json_object_object_length(jobj_);
}


string Object::getString() const noexcept
{
    return ::json_object_to_json_string(jobj_);
}


string Object::getStringExt(int flags) const noexcept
{
    return ::json_object_to_json_string_ext(jobj_, flags);
}

}

