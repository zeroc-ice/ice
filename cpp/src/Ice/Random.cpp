// Copyright (c) ZeroC, Inc.

#include "Random.h"

#include <cassert>

using namespace std;

void
IceInternal::generateRandom(char* buffer, size_t size)
{
    // We use the random_device directly here to get cryptographic random numbers when possible.
    thread_local static std::random_device rd;
    uniform_int_distribution<unsigned int> distribution(0, 255);
    for (size_t i = 0; i < size; ++i, ++buffer)
    {
        *buffer = static_cast<char>(distribution(rd));
    }
}

unsigned int
IceInternal::random(unsigned int limit)
{
    assert(limit > 0);
    thread_local static std::random_device rd;
    thread_local static std::mt19937 rng(rd());
    uniform_int_distribution<unsigned int> distribution(0, limit - 1);
    return distribution(rng);
}
