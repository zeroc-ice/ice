//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_RANDOM_H
#define ICE_UTIL_RANDOM_H

#include <IceUtil/Config.h>
#include <IceUtil/Exception.h>

#include <algorithm>
#include <random>

namespace IceUtilInternal
{

ICE_API void generateRandom(char*, size_t);
ICE_API unsigned int random(int = 0);

template<class T>
void shuffle(T first, T last)
{
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(first, last, rng);
}

}

#endif
