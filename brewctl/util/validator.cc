/*
    validator.cc: provides various validation predicates, e.g. for use with the Config::get<T>() method.

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/util/validator.h"

using std::string;


namespace Util::Validator
{
    // ge0() - returns true if <val> is greater than or equal to zero
    //
    bool ge0(const short& val)              noexcept { return val >= 0;     }
    bool ge0(const int& val)                noexcept { return val >= 0;     }
    bool ge0(const long& val)               noexcept { return val >= 0;     }
    bool ge0(const long long& val)          noexcept { return val >= 0;     }
    bool ge0(const float& val)              noexcept { return val >= 0.0;   }
    bool ge0(const double& val)             noexcept { return val >= 0.0;   }
    bool ge0(const long double& val)        noexcept { return val >= 0.0;   }

    // gt0() - returns true if <val> is greater than zero
    //
    bool gt0(const short& val)              noexcept { return val > 0;      }
    bool gt0(const int& val)                noexcept { return val > 0;      }
    bool gt0(const long& val)               noexcept { return val > 0;      }
    bool gt0(const long long& val)          noexcept { return val > 0;      }
    bool gt0(const unsigned short& val)     noexcept { return val > 0;      }
    bool gt0(const unsigned int& val)       noexcept { return val > 0;      }
    bool gt0(const unsigned long& val)      noexcept { return val > 0;      }
    bool gt0(const unsigned long long& val) noexcept { return val > 0;      }
    bool gt0(const float& val)              noexcept { return val > 0.0;    }
    bool gt0(const double& val)             noexcept { return val > 0.0;    }
    bool gt0(const long double& val)        noexcept { return val > 0.0;    }

    // notEmpty() - returns true if <val> (a string) is non-empty
    //
    bool notEmpty(const string& val)        noexcept { return val.length(); }

} // namespace Util::Validator

