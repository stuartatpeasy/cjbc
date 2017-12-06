#ifndef INCLUDE_SERVICE_JSON_JSON_H_INC
#define INCLUDE_SERVICE_JSON_JSON_H_INC
/*
    json.h: wrapper around libjson-c functions

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include <cstdint>
#include <string>

extern "C"
{
#include <json-c/json.h>
}


class Json
{
public:
                            Json() noexcept;
                            Json(const Json& rhs) = delete;
                            Json(Json&& rhs) noexcept;
    virtual                 ~Json() noexcept;

    virtual Json&           operator=(const Json& rhs) = delete;
    virtual Json&           operator=(Json&& rhs) noexcept;

                            operator std::string() noexcept;

    enum json_type          type() noexcept;
    void                    addRef() noexcept;
    void                    decRef() noexcept;
    std::string             toString() noexcept;

    struct json_object *    rawPtr() noexcept { return jobj_; };

protected:
    virtual Json&           move(Json& rhs) noexcept;

    struct json_object *    jobj_;
};

#endif // INCLUDE_SERVICE_JSON_JSON_H_INC

