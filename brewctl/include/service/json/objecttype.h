#ifndef INCLUDE_SERVICE_JSON_OBJECTTYPE_H_INC
#define INCLUDE_SERVICE_JSON_OBJECTTYPE_H_INC
/*
    objecttype.h: wrapper around the type-specific libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/object.h"


namespace JSON
{

template<typename T>
class ObjectType : public Object
{
            ObjectType<T>() noexcept;
            ObjectType<T>(const T& val) noexcept;
    T       get() const noexcept;

private:
    T       val_;
};

}

#endif // INCLUDE_SERVICE_JSON_OBJECTTYPE_H_INC

