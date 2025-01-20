// Copyright (c) ZeroC, Inc.

#ifndef ICE_RANDOM_H
#define ICE_RANDOM_H

#include "Ice/Config.h"
#include <algorithm>
#include <random>

namespace IceInternal
{
    ICE_API void generateRandom(char*, size_t);
    ICE_API unsigned int random(unsigned int = 0);

    template<class T> void shuffle(T first, T last)
    {
        thread_local static std::random_device rd;
        thread_local static std::mt19937 rng(rd());
        std::shuffle(first, last, rng);
    }
}

#endif
