/*
    array.cc: wrapper around libjson-c array functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/array.h"
#include "include/framework/log.h"
#include "include/service/json/element.h"
#include "include/service/json/object.h"
#include <cstdlib>      // NULL
#include <utility>

using std::string;


// default ctor
//
JsonArray::JsonArray() noexcept
{
    jobj_ = ::json_object_new_array();
}


// move ctor
//
JsonArray::JsonArray(JsonArray&& rhs) noexcept
    : Json(std::move(rhs))
{
    move(rhs);
}


// dtor - free contained objects
//
JsonArray::~JsonArray()
{
    for(auto it = array_.begin(); it != array_.end(); ++it)
        delete *it;
}


// move-assignment operator
//
JsonArray& JsonArray::operator=(JsonArray&& rhs) noexcept
{
    Json::operator=(std::move(rhs));

    return move(rhs);
}


// append() - append the item specified by <val> to the array.
//
JsonArray& JsonArray::append(Json * val) noexcept
{
    val->addRef();
    array_.push_back(val);
    ::json_object_array_add(jobj_, val->rawPtr());

    return *this;
}


// getAt() - get a pointer to the item at <index> from the array.  If <index> is out-of-bounds, return NULL.
//
Json * JsonArray::getAt(const int index) noexcept
{
    if((jobj_ == NULL) || (index < 0) || (index >= length()))
        return NULL;

    return array_[index];
}


// length() - return the number of elements in the array.
//
int JsonArray::length() noexcept
{
    return (jobj_ != NULL) ? ::json_object_array_length(jobj_) : 0;
}


// move() - helper method for move ctor and move-assignment operator
//
JsonArray& JsonArray::move(JsonArray& rhs) noexcept
{
    array_.swap(rhs.array_);

    return *this;
}

// static fromPtr() - construct a JSON array from the struct json_object ptr supplied in <jobj>, recursing as required
// in order to build a full tree of class Json-derived objects.
//
JsonArray *JsonArray::fromPtr(struct json_object *jobj) noexcept
{
    if(jobj == NULL)
    {
        logWarning("JsonArray::fromPtr(): called with NULL ptr");
        return NULL;
    }

    if(::json_object_get_type(jobj) != json_type_array)
    {
        logWarning("JsonArray::fromPtr(): called with something that is not a JSON array");
        return NULL;
    }

    JsonArray *ret = new JsonArray();
    ret->jobj_ = jobj;

    for(int i = 0; i < ::json_object_array_length(jobj); ++i)
    {
        struct json_object *val = ::json_object_array_get_idx(jobj, i);
        if(val == NULL)
        {
            logWarning("JsonArray::fromPtr(): index %d of supplied array is NULL; ignoring");
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
                logWarning("JsonArray::fromPtr(): json_object_object_foreach() returned a val with unknown type for "
                           "index %d; ignoring", i);
                continue;
        }

        if(child != NULL)
            ret->array_.push_back(child);
        else
            logWarning("JsonArray::fromPtr(): failed to construct child object for index %d; ignoring", i);
    }

    return ret;
}

