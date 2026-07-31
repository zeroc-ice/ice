// Copyright (c) ZeroC, Inc.

#ifndef ICE_RANDOM_H
#define ICE_RANDOM_H

#include "Ice/Config.h"
#include <algorithm>
#include <random>

namespace IceInternal
{
    // Fills the buffer with cryptographically strong random bytes from the operating system's CSPRNG.
    ICE_API void generateRandom(char*, size_t);

    ICE_API unsigned int random(unsigned int = 0);

    // Creates a std::mt19937 engine seeded from the operating system's CSPRNG.
    ICE_API std::mt19937 createMT19937();

    template<class T> void shuffle(T first, T last)
    {
        thread_local static std::mt19937 rng = createMT19937();
        std::shuffle(first, last, rng);
    }
}

#endif
