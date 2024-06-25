//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_STRING_CONVERTER_H
#define ICE_UTIL_STRING_CONVERTER_H

#include "Ice/Config.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace IceUtilInternal
{
    //
    // Convert from UTF-8 to UTF-16/32
    //
    ICE_API std::vector<unsigned short> toUTF16(const std::vector<std::uint8_t>&);
    ICE_API std::vector<unsigned int> toUTF32(const std::vector<std::uint8_t>&);

    //
    // Convert from UTF-32 to UTF-8
    //
    ICE_API std::vector<std::uint8_t> fromUTF32(const std::vector<unsigned int>&);
}

#endif
