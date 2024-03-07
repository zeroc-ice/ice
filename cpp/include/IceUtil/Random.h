//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_RANDOM_H
#define ICE_UTIL_RANDOM_H

#include <IceUtil/Config.h>

#include <algorithm>
#include <random>

namespace IceUtilInternal
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
