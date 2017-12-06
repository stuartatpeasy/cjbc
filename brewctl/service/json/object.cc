/*
    object.cc: wrapper around libjson-c object functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/object.h"
#include "include/framework/log.h"
#include <cstdlib>      // NULL
#include <utility>

using std::string;


// default ctor
//
JsonObject::JsonObject() noexcept
{
    jobj_ = ::json_object_new_object();
}


// move ctor
//
JsonObject::JsonObject(JsonObject&& rhs) noexcept
    : Json(std::move(rhs))
{
    move(rhs);
}


// dtor - free contained objects
//
JsonObject::~JsonObject() noexcept
{
    for(auto it = map_.begin(); it != map_.end(); ++it)
        delete it->second;
}


// move-assignment operator
//
JsonObject& JsonObject::operator=(JsonObject&& rhs) noexcept
{
    Json::operator=(std::move(rhs));
    return move(rhs);
}


// add() - add a new field <val>, named <key>, to the current object.  If a field with a matching name already exists in
// the object, decrement its reference count (possibly freeing it), before replacing it with the new field.
//
JsonObject& JsonObject::add(const string& key, Json *val) noexcept
{
    auto item = map_.find(key);
    if(item != map_.end())
        item->second->decRef();

    val->addRef();
    map_[key] = val;
    ::json_object_object_add(jobj_, key.c_str(), val->rawPtr());

    return *this;
}


// get() - get the field named by <key> without incrementing its reference count.  If no such field exists, return NULL.
//
Json *JsonObject::get(const string& key) noexcept
{
    auto item = map_.find(key);
    if(item == map_.end())
        return NULL;

    return map_[key];
}


// length() - return the number of fields contained in this JSON object.
//
int JsonObject::length() noexcept
{
    return (jobj_ != NULL) ? ::json_object_object_length(jobj_) : 0;
}


// move() - helper method for move ctor and move-assignment operator.
//
JsonObject& JsonObject::move(JsonObject& rhs) noexcept
{
    map_.swap(rhs.map_);

    return *this;
}

