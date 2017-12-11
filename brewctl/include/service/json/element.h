#ifndef INCLUDE_SERVICE_JSON_ELEMENT_H_INC
#define INCLUDE_SERVICE_JSON_ELEMENT_H_INC
/*
    element.h: wrapper around libjson-c element-specific functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/service/json/json.h"
#include <cstdint>
#include <string>


class JsonElement : public Json
{
public:
                            JsonElement() = delete;
                            JsonElement(const int32_t& val) noexcept;
                            JsonElement(const int64_t& val) noexcept;
                            JsonElement(const bool& val) noexcept;
                            JsonElement(const double& val) noexcept;
                            JsonElement(const std::string& val) noexcept;

                            JsonElement(const JsonElement& rhs) = delete;

    virtual                 ~JsonElement() = default;

    template<typename T> T  get() noexcept;

    static JsonElement *    fromPtr(struct json_object *jobj) noexcept;

private:
                            JsonElement(struct json_object *jobj) noexcept;
};

#endif // INCLUDE_SERVICE_JSON_ELEMENT_H_INC

