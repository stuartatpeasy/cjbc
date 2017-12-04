#ifndef STRINGVALUE_H_INC
#define STRINGVALUE_H_INC
/*
    stringvalue: container to hold a std::string, and allow it to be converted to other values.  Helper for templated
    data-retrieval methods in classes like Config and SQLiteColumn.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include <cstdlib>
#include <string>


class StringValue
{
public:
                    StringValue() noexcept
                    { };

                    StringValue(const std::string& value) noexcept
                        : value_(value) { };

                    StringValue(const char * const buf, const size_t len) noexcept
                    {
                        set(buf, len);
                    };

                    StringValue(const StringValue& rhs) noexcept
                        : value_(rhs.value_) { };

                    StringValue(StringValue&& rhs) noexcept
                        : value_(rhs.value_)
                    {
                        rhs.value_ = "";
                    };

    StringValue&    operator=(const StringValue& rhs) noexcept
                    {
                        value_ = rhs.value_;
                        return *this;
                    };

    StringValue&    operator==(StringValue&& rhs) noexcept
                    {
                        value_ = rhs.value_;
                        rhs.value_ = "";
                        return *this;
                    };

                    // str() - retrieve the underlying std::string object
                    //
    std::string     str() const noexcept
                    {
                        return value_;
                    };

                    // set() - copy data from a buffer into the object, possibly including embedded NUL characters.
                    //
    StringValue&    set(const char * const buf, const size_t len) noexcept
                    {
                        clear();
                        value_.resize(len + 1);
                        value_.assign(buf, len);
                        return *this;
                    };

                    // clear() - reset the contents of the object
                    //
    void            clear() noexcept
                    {
                        value_.clear();
                    };

                    // Type conversion operators
                    //
                    operator short()                const noexcept  { return ::strtol(value_.c_str(), NULL, 0);     };
                    operator int()                  const noexcept  { return ::strtol(value_.c_str(), NULL, 0);     };
                    operator long()                 const noexcept  { return ::strtol(value_.c_str(), NULL, 0);     };
                    operator long long()            const noexcept  { return ::strtoll(value_.c_str(), NULL, 0);    };
                    operator unsigned short()       const noexcept  { return ::strtoul(value_.c_str(), NULL, 0);    };
                    operator unsigned int()         const noexcept  { return ::strtoul(value_.c_str(), NULL, 0);    };
                    operator unsigned long()        const noexcept  { return ::strtoul(value_.c_str(), NULL, 0);    };
                    operator unsigned long long()   const noexcept  { return ::strtoull(value_.c_str(), NULL, 0);   };
                    operator float()                const noexcept  { return ::strtof(value_.c_str(), NULL);        };
                    operator double()               const noexcept  { return ::strtod(value_.c_str(), NULL);        };
                    operator long double()          const noexcept  { return ::strtold(value_.c_str(), NULL);       };
                    operator std::string()          const noexcept  { return value_;                                };

private:
    std::string     value_;
};

#endif // STRINGVALUE_H_INC

