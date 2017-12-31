#ifndef STRINGVALUE_H_INC
#define STRINGVALUE_H_INC
/*
    stringvalue: container to hold a std::string, or a string representation of another value, and allow it to be
    converted to other values.  Helper for templated data-retrieval methods in classes like Config and SQLiteColumn.
    This is absolutely "that class", i.e. the one everybody wants to write but knows they really shouldn't.

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include <cstdlib>
#include <cstring>
#include <string>


class StringValue
{
public:
                        StringValue() noexcept
                        { };

                        StringValue(const std::string& val) noexcept
                            : value_(val) { };

                        StringValue(const char * const buf, const size_t len) noexcept
                        {
                            set(buf, len);
                        };

                        StringValue(const short& val)               noexcept { fmt<short>("%hd", val);               }
                        StringValue(const int& val)                 noexcept { fmt<int>("%d", val);                  }
                        StringValue(const long& val)                noexcept { fmt<long>("%ld", val);                }
                        StringValue(const long long& val)           noexcept { fmt<long long>("%lld", val);          }
                        StringValue(const unsigned short& val)      noexcept { fmt<unsigned short>("%hu", val);      }
                        StringValue(const unsigned int& val)        noexcept { fmt<unsigned int>("%u", val);         }
                        StringValue(const unsigned long& val)       noexcept { fmt<unsigned long>("%lu", val);       }
                        StringValue(const unsigned long long& val)  noexcept { fmt<unsigned long long>("%llu", val); }
                        StringValue(const float& val)               noexcept { fmt<float>("%f", val);                }
                        StringValue(const double& val)              noexcept { fmt<double>("%lf", val);              }
                        StringValue(const long double& val)         noexcept { fmt<long double>("%llf", val);        }

                        StringValue(const StringValue& rhs) noexcept
                            : value_(rhs.value_) { };

                        StringValue(StringValue&& rhs) noexcept
                            : value_(rhs.value_)
                        {
                            rhs.value_ = "";
                        };

    StringValue&        operator=(const StringValue& rhs) noexcept
                        {
                            value_ = rhs.value_;
                            return *this;
                        };

    StringValue&        operator==(StringValue&& rhs) noexcept
                        {
                            value_ = rhs.value_;
                            rhs.value_ = "";
                            return *this;
                        };

                        // str() - retrieve the underlying std::string object
                        //
    std::string         str() const noexcept
                        {
                            return value_;
                        };

                        // set() - copy data from a buffer into the object, possibly including embedded NUL characters.
                        //
    StringValue&        set(const char * const buf, size_t len = -1) noexcept
                        {
                            if(len == (size_t) -1)
                                len = ::strlen(buf);

                            clear();
                            value_.resize(len + 1);
                            value_.assign(buf, len);
                            return *this;
                        };

                        // clear() - reset the contents of the object
                        //
    void                clear() noexcept
                        {
                            value_.clear();
                        };

                        // Type conversion operators
                        //
                        operator short()                const noexcept  { return strtol();      };
                        operator int()                  const noexcept  { return strtol();      };
                        operator long()                 const noexcept  { return strtol();      };
                        operator long long()            const noexcept  { return strtoll();     };
                        operator unsigned short()       const noexcept  { return strtoul();     };
                        operator unsigned int()         const noexcept  { return strtoul();     };
                        operator unsigned long()        const noexcept  { return strtoul();     };
                        operator unsigned long long()   const noexcept  { return strtoull();    };
                        operator float()                const noexcept  { return strtof();      };
                        operator double()               const noexcept  { return strtod();      };
                        operator long double()          const noexcept  { return strtold();     };
                        operator std::string()          const noexcept  { return value_;        };

    long                strtol()    const noexcept  { return ::strtol(value_.c_str(), NULL, 0);     };
    long long           strtoll()   const noexcept  { return ::strtoll(value_.c_str(), NULL, 0);    };
    unsigned long       strtoul()   const noexcept  { return ::strtoul(value_.c_str(), NULL, 0);    };
    unsigned long long  strtoull()  const noexcept  { return ::strtoull(value_.c_str(), NULL, 0);   };
    float               strtof()    const noexcept  { return ::strtof(value_.c_str(), NULL);        };
    double              strtod()    const noexcept  { return ::strtod(value_.c_str(), NULL);        };
    long double         strtold()   const noexcept  { return ::strtold(value_.c_str(), NULL);       };

private:
    template<typename T>
    void                fmt(const char * const fmtstr, const T& val) noexcept
                        {
                            char buf[64];
                            ::sprintf(buf, fmtstr, val);
                            set(buf);
                        }

    std::string         value_;
};

#endif // STRINGVALUE_H_INC

