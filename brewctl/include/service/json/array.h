#ifndef INCLUDE_SERVICE_JSON_ARRAY_H_INC
#define INCLUDE_SERVICE_JSON_ARRAY_H_INC
/*
    array.h: wrapper around libjson-c array functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/json.h"
#include <cstdint>
#include <string>
#include <vector>


class JsonArray : public Json
{
typedef std::vector<Json *> JsonPtrArray_t;

public:
                            JsonArray() noexcept;
                            JsonArray(const JsonArray& rhs) = delete;
                            JsonArray(JsonArray&& rhs) noexcept;
    virtual                 ~JsonArray() noexcept;

    virtual JsonArray&      operator=(const JsonArray& rhs) = delete;
    virtual JsonArray&      operator=(JsonArray&& rhs) noexcept;

    JsonArray&              append(Json * val) noexcept;
    Json *                  getAt(const int index) noexcept;
    int                     length() noexcept;

    static JsonArray *      fromPtr(struct json_object *jobj) noexcept;

protected:
    virtual JsonArray&      move(JsonArray& rhs) noexcept;

    JsonPtrArray_t          array_;
};

#endif // INCLUDE_SERVICE_JSON_ARRAY_H_INC

