/*
    array.cc: wrapper around libjson-c array functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/array.h"
#include "include/framework/log.h"
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

