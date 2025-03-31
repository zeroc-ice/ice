// Copyright (c) ZeroC, Inc.

#ifndef ICE_ENDPOINT_SELECTION_TYPE_H
#define ICE_ENDPOINT_SELECTION_TYPE_H

#include <cstdint>

namespace Ice
{
    /// Determines how the Ice runtime sorts proxy endpoints when establishing a connection.
    enum class EndpointSelectionType : std::uint8_t
    {
        /// The Ice runtime shuffles the endpoints in a random order.
        Random,

        /// The Ice runtime uses the endpoints in the order they appear in the proxy.
        Ordered
    };
}

#endif
