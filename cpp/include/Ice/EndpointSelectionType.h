// Copyright (c) ZeroC, Inc.

#ifndef ICE_ENDPOINT_SELECTION_TYPE_H
#define ICE_ENDPOINT_SELECTION_TYPE_H

#include <cstdint>

namespace Ice
{
    /// Determines the order in which the Ice run time uses the endpoints in a proxy when establishing a connection.
    /// \headerfile Ice/Ice.h
    enum class EndpointSelectionType : std::uint8_t
    {
        /// <code>Random</code> causes the endpoints to be arranged in a random order.
        Random,
        /// <code>Ordered</code> forces the Ice run time to use the endpoints in the order they appeared in the proxy.
        Ordered
    };
}

#endif
