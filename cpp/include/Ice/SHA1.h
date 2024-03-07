//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SHA1_H
#define ICE_SHA1_H

#include "Config.h"

#include <memory>
#include <vector>

namespace IceInternal
{

ICE_API void
sha1(const unsigned char*, std::size_t, std::vector<unsigned char>&);

ICE_API void
sha1(const std::byte*, std::size_t, std::vector<std::byte>&);

class ICE_API SHA1
{
public:

    SHA1();
    ~SHA1();

    void update(const unsigned char*, std::size_t);
    void finalize(std::vector<unsigned char>&);
    void finalize(std::vector<std::byte>&);

private:

    // noncopyable
    SHA1(const SHA1&);
    SHA1 operator=(const SHA1&);

    class Hasher;
    std::unique_ptr<Hasher> _hasher;
};

}
#endif
