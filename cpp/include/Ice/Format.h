// Copyright (c) ZeroC, Inc.

#ifndef ICE_FORMAT_H
#define ICE_FORMAT_H

#include <cstdint>

namespace Ice
{
    /// Specifies the format for marshaling classes and exceptions with the Ice 1.1 encoding.
    enum class FormatType : std::uint8_t
    {
        /// Favors compactness, but does not support slicing-off unknown slices during unmarshaling.
        CompactFormat,

        /// Allows slicing-off unknown slices during unmarshaling, at the cost of some extra space in the marshaled
        /// data.
        SlicedFormat
    };
}

#endif
