/*
    random.cc: utility functions relating to random numbers

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include "include/util/random.h"
#include <chrono>
#include <random>

using std::chrono::high_resolution_clock;


namespace Util
{
namespace Random
{

static unsigned int seedVal_ = 0;


// seed() - seed the random-number generator with the specified <seedVal>.
//
void seed(unsigned int seedVal) noexcept
{
    seedVal_ = seedVal;
}


// seed() - seed the random-number generator with the current time from the high-resolution clock
//
void seed() noexcept
{
    seed(high_resolution_clock::now().time_since_epoch().count());
}


// randomUnsignedInt() - return a random unsigned integer.
//
unsigned int randomUnsignedInt() noexcept
{
    std::minstd_rand0 generator(seedVal_);
    unsigned int ret = generator();

    seedVal_ = ret;

    return ret;
}

} // namespace Random
} // namespace Util

