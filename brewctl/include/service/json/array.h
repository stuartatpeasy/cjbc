#ifndef INCLUDE_SERVICE_JSON_ARRAY_H_INC
#define INCLUDE_SERVICE_JSON_ARRAY_H_INC
/*
    array.h: wrapper around the array-specific libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/object.h"


namespace JSON
{

class Array : public Object
{
public:
            Array() noexcept;
};

} // namespace JSON

#endif // INCLUDE_SERVICE_JSON_ARRAY_H_INC

