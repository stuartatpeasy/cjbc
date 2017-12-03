#ifndef INCLUDE_SERVICE_JSON_OBJECT_H_INC
#define INCLUDE_SERVICE_JSON_OBJECT_H_INC
/*
    object.h: wrapper around the libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include <json-c/json.h>
#include <string>


namespace JSON
{

class Object
{
public:
    typedef enum Type
    {
        OBJECT,
        ARRAY,
        INT32,
        INT64,
        STRING,
        BOOL,
        DOUBLE
    } Type_t;

                    Object() noexcept;
    virtual         ~Object() = default;

                    operator std::string() const noexcept { return getString(); };

    int             length() const noexcept;
    std::string     getString() const noexcept;
    std::string     getStringExt(int flags) const noexcept;
    Type_t          type() const noexcept { return type_; };

protected:
    Type_t          type_;
    json_object *   jobj_;
    
};

}

#endif // INCLUDE_SERVICE_JSON_OBJECT_H_INC

