/*
    json.cc: wrapper around libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/json.h"
#include "include/framework/log.h"
#include <cstdlib>      // NULL
#include <utility>

using std::string;


// default ctor
//
Json::Json() noexcept
    : jobj_(NULL)
{
}


// move ctor
//
Json::Json(Json&& rhs) noexcept
{
    move(rhs);
}


// dtor - decrement the object's reference count, possibly deallocating it.
//
Json::~Json() noexcept
{
    decRef();
}


// move ctor
//
Json& Json::operator=(Json&& rhs) noexcept
{
    return move(rhs);
}


// operator string() - obtain a string representation of the object.
//
Json::operator string() noexcept
{
    return toString();
}


// move() - helper method for move ctor and move-assignment operator.
//
Json& Json::move(Json& rhs) noexcept
{
    jobj_ = rhs.jobj_;
    rhs.jobj_ = NULL;

    return *this;
}


// type() - get the type of the data item stored in this object.
//
enum json_type Json::type() noexcept
{
    return (jobj_ != NULL) ? ::json_object_get_type(jobj_) : json_type_null;
}


// addRef() - increment the reference count of the contained JSON object.
//
void Json::addRef() noexcept
{
    if(jobj_ != NULL)
        ::json_object_get(jobj_);
    else
        logWarning("Json::addRef() called on NULL object");
}


// decRef() - decrement the reference count of the contained JSON object.  This may result in the object being freed, in
// which case jobj_ will be set to NULL.
//
void Json::decRef() noexcept
{
    if(jobj_ != NULL)
    {
        if(::json_object_put(jobj_) == 1)
        {
            logDebug("Json::decRef() - freed JSON object at %p", jobj_);
            jobj_ = NULL;
        }
    }
    else
        logWarning("Json::decRef() called on NULL object");

}


// toString() - obtain a string representation of the contained JSON object.
//
string Json::toString() noexcept
{
    return (jobj_ != NULL) ? ::json_object_to_json_string(jobj_) : "";
}

