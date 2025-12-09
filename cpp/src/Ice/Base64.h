// Copyright (c) ZeroC, Inc.

#ifndef ICE_BASE_64_H
#define ICE_BASE_64_H

#include "Ice/Config.h"

#include <string>
#include <vector>

namespace IceInternal
{
    class Base64
    {
    public:
        static std::string encode(const std::vector<std::byte>&);
        static std::vector<std::byte> decode(const std::string&);
        static bool isBase64(char);

    private:
        static char encode(std::byte);
        static std::byte decode(char);
    };
}

#endif
