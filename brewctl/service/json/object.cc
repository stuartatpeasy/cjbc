/*
    object.cc: wrapper around libjson-c object functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/object.h"
#include "include/framework/log.h"
#include "include/service/json/array.h"
#include "include/service/json/element.h"
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


// static fromPtr() - construct a JSON object from the struct json_object ptr supplied in <jobj>, recursing as required
// in order to build a full tree of class Json-derived objects.
//
JsonObject *JsonObject::fromPtr(struct json_object *jobj) noexcept
{
    if(jobj == NULL)
    {
        logWarning("JsonObject::fromPtr(): called with NULL ptr");
        return NULL;
    }

    if(::json_object_get_type(jobj) != json_type_object)
    {
        logWarning("JsonObject::fromPtr(): called with something that is not a JSON object");
        return NULL;
    }

    JsonObject *ret = new JsonObject();
    ret->jobj_ = jobj;

    json_object_object_foreach(jobj, key, val)
    {
        if(val == NULL)
        {
            logWarning("JsonObject::fromPtr(): json_object_object_foreach() returned NULL val for key '%s'; ignoring",
                        key);
            continue;
        }

        Json *child = NULL;
        switch(::json_object_get_type(val))
        {
            case json_type_object:
                child = JsonObject::fromPtr(val);
                break;

            case json_type_array:
                child = JsonArray::fromPtr(val);
                break;

            case json_type_boolean:
            case json_type_int:
            case json_type_double:
            case json_type_string:
                child = JsonElement::fromPtr(val);
                break;

            default:
                logWarning("JsonObject::fromPtr(): json_object_object_foreach() returned a val with unknown type for "
                           "key '%s'; ignoring", key);
                continue;
        }

        if(child != NULL)
            ret->map_[key] = child;
        else
            logWarning("JsonObject::fromPtr(): failed to construct child object for key '%s'; ignoring", key);
    }

    return ret;
}

