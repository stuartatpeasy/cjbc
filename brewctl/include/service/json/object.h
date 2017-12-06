#ifndef INCLUDE_SERVICE_JSON_OBJECT_H_INC
#define INCLUDE_SERVICE_JSON_OBJECT_H_INC
/*
    object.h: wrapper around libjson-c object functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/json.h"
#include <cstdint>
#include <map>
#include <string>


class JsonObject : public Json
{
typedef std::map<std::string, Json *> JsonPtrMap_t;

public:
                            JsonObject() noexcept;
                            JsonObject(const JsonObject& rhs) = delete;
                            JsonObject(JsonObject&& rhs) noexcept;
    virtual                 ~JsonObject() noexcept;

    virtual JsonObject&     operator=(const JsonObject& rhs) = delete;
    virtual JsonObject&     operator=(JsonObject&& rhs) noexcept;

    JsonObject&             add(const std::string& key, Json * val) noexcept;
    Json *                  get(const std::string& key) noexcept;
    int                     length() noexcept;

protected:
    virtual JsonObject&     move(JsonObject& rhs) noexcept;

    JsonPtrMap_t            map_;
};

#endif // INCLUDE_SERVICE_JSON_OBJECT_H_INC

