#ifndef INCLUDE_SERVICE_JSONOBJECTARRAY_H_INC
#define INCLUDE_SERVICE_JSONOBJECTARRAY_H_INC
/*
    jsonobjectarray.h: wrapper around the array-specific libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/jsonobject.h"


class JsonObjectArray : public JsonObject
{
public:
            JsonObjectArray() noexcept;
};

#endif // INCLUDE_SERVICE_JSONOBJECTARRAY_H_INC

