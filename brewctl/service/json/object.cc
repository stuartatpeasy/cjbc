/*
    object.cc: wrapper around the libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/object.h"
#include "include/service/json/array.h"

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


Object::~Object() noexcept
{
    ::json_object_put(jobj_);
}


// length() - get the storage size of the object
//
int Object::length() const noexcept
{
    return ::json_object_object_length(jobj_);
}


// getString() - get the object and its descendants as a JSON string
//
string Object::getString() const noexcept
{
    return ::json_object_to_json_string(jobj_);
}


// getStringExt() - get the object and its descendants as a JSON string, with options specified in <flags>
//
string Object::getStringExt(int flags) const noexcept
{
    return ::json_object_to_json_string_ext(jobj_, flags);
}


// addObject() - add an object field to this object
//
Object& Object::addObject(const std::string& key, Object obj) noexcept
{
    ::json_object_object_add(jobj_, key.c_str(), obj.jobj_);
    return *this;
}


// addArray() - add an array field to this object
//
Object& Object::addArray(Array array) noexcept
{
    ::json_object_array_add(jobj_, array.jobj_);
    return *this;
}

} // namespace JSON

