#ifndef INCLUDE_SERVICE_JSONOBJECT_H_INC
#define INCLUDE_SERVICE_JSONOBJECT_H_INC
/*
    jsonobject.h: wrapper around the libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include <json-c/json.h>
#include <string>


class JsonObject
{
public:
    typedef enum JsonType
    {
        OBJECT,
        ARRAY,
        INT32,
        INT64,
        STRING,
        BOOL,
        DOUBLE
    } JsonType_t;

                    JsonObject() noexcept;
    virtual         ~JsonObject() = default;

                    operator std::string() const noexcept { return getString(); };

    int             length() const noexcept;
    std::string     getString() const noexcept;
    std::string     getStringExt(int flags) const noexcept;
    JsonType_t      type() const noexcept { return type_; };

protected:
    JsonType_t      type_;
    json_object *   jobj_;
    
};

#endif // INCLUDE_SERVICE_JSONOBJECT_H_INC

