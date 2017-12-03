#ifndef INCLUDE_SERVICE_JSONOBJECTTYPE_H_INC
#define INCLUDE_SERVICE_JSONOBJECTTYPE_H_INC
/*
    jsonobjecttype.h: wrapper around the type-specific libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "jsonobject.h"


template<typename T>
class JsonObjectType : public JsonObject
{
            JsonObjectType<T>() noexcept;
            JsonObjectType<T>(const T& val) noexcept;
    T       get() const noexcept;

private:
    T       val_;
};

#endif // INCLUDE_SERVICE_JSONOBJECTTYPE_H_INC

