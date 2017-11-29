#ifndef STRINGVALUE_H_INC
#define STRINGVALUE_H_INC
/*
    stringvalue: container to hold a std::string, and allow it to be converted to other values.  Helper for templated
    data-retrieval methods in classes like Config and SQLiteColumn.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

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
                        value_.append(buf, len).append('\0');
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
                    operator short()                const noexcept  { return std::stol(value_);     };
                    operator int()                  const noexcept  { return std::stol(value_);     };
                    operator long()                 const noexcept  { return std::stol(value_);     };
                    operator long long()            const noexcept  { return std::stoll(value_);    };
                    operator unsigned short()       const noexcept  { return std::stoul(value_);    };
                    operator unsigned int()         const noexcept  { return std::stoul(value_);    };
                    operator unsigned long()        const noexcept  { return std::stoul(value_);    };
                    operator unsigned long long()   const noexcept  { return std::stoull(value_);   };
                    operator float()                const noexcept  { return std::stof(value_);     };
                    operator double()               const noexcept  { return std::stod(value_);     };
                    operator long double()          const noexcept  { return std::stold(value_);    };
                    operator std::string()          const noexcept  { return value_;                };

private:
    std::string     value_;
};

#endif // STRINGVALUE_H_INC

