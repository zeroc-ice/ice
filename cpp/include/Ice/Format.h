// Copyright (c) ZeroC, Inc.

#ifndef ICE_FORMAT_H
#define ICE_FORMAT_H

#include <cstdint>

namespace Ice
{
    /**
     * Describes the possible formats for classes and exceptions.
     */
    enum class FormatType : std::uint8_t
    {
        /**
         * A minimal format that eliminates the possibility for slicing unrecognized types.
         */
        CompactFormat,
        /**
         * Allow slicing and preserve slices for unknown types.
         */
        SlicedFormat
    };
}

#endif
