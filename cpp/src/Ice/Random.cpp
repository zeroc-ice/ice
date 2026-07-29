// Copyright (c) ZeroC, Inc.

#ifdef _WIN32
#    define _CRT_RAND_S
#endif

#include "Random.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <system_error>

#if defined(__linux__)
#    include <cerrno>
#    include <sys/random.h>
#endif

using namespace std;

// Generate the random bytes with the operating system's CSPRNG, never with std::random_device: the C++
// standard gives std::random_device no cryptographic guarantee, and some implementations draw from raw CPU
// entropy instructions whose output defective hardware can make predictable.
void
IceInternal::generateRandom(char* buffer, size_t size)
{
#if defined(_WIN32)
    // rand_s is backed by the same system CSPRNG as BCryptGenRandom and requires no additional link
    // dependency.
    size_t index = 0;
    while (index < size)
    {
        unsigned int r = 0;
        errno_t err = rand_s(&r);
        if (err != 0)
        {
            throw system_error(err, generic_category(), "rand_s failed");
        }
        size_t n = min(sizeof(r), size - index);
        memcpy(buffer + index, &r, n);
        index += n;
    }
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    arc4random_buf(buffer, size);
#elif defined(__linux__)
    // getrandom with flags == 0 deliberately blocks until the kernel's entropy pool is initialized; don't
    // "fix" this with GRND_NONBLOCK. There is also deliberately no /dev/urandom fallback here: getrandom is
    // available in every supported glibc (2.25+), so a failure is a real error, not a missing feature.
    size_t index = 0;
    while (index < size)
    {
        ssize_t n = getrandom(buffer + index, size - index, 0);
        if (n <= 0)
        {
            if (n < 0 && errno == EINTR)
            {
                continue;
            }
            if (n == 0)
            {
                throw system_error(EIO, generic_category(), "getrandom returned no data");
            }
            throw system_error(errno, generic_category(), "getrandom failed");
        }
        index += static_cast<size_t>(n);
    }
#else
#    error "unsupported platform: no OS CSPRNG binding"
#endif
}

unsigned int
IceInternal::random(unsigned int limit)
{
    assert(limit > 0);
    thread_local static mt19937 rng = createMT19937();
    uniform_int_distribution<unsigned int> distribution(0, limit - 1);
    return distribution(rng);
}

mt19937
IceInternal::createMT19937()
{
    array<uint32_t, 8> seedData;
    generateRandom(reinterpret_cast<char*>(seedData.data()), sizeof(seedData));
    seed_seq seq(seedData.begin(), seedData.end());
    return mt19937(seq);
}
