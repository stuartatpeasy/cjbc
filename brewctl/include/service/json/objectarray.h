#ifndef INCLUDE_SERVICE_JSON_OBJECTARRAY_H_INC
#define INCLUDE_SERVICE_JSON_OBJECTARRAY_H_INC
/*
    objectarray.h: wrapper around the array-specific libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/object.h"


namespace JSON
{

class ObjectArray : public Object
{
public:
            ObjectArray() noexcept;
};

} // namespace JSON

#endif // INCLUDE_SERVICE_JSON_OBJECTARRAY_H_INC

