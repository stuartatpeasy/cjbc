#ifndef UTIL_RANDOM_H_INC
#define UTIL_RANDOM_H_INC
/*
    random.h: utility functions related to random numbers

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/framework/error.h"


namespace Util::Random
{
    void                        seed(unsigned int seedVal) noexcept;
    void                        seed() noexcept;
    unsigned int                randomUnsignedInt() noexcept;
} // namespace Util::Random

#endif // UTIL_RANDOM_H_INC

