//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ENDPOINT_SELECTION_TYPE_H
#define ICE_ENDPOINT_SELECTION_TYPE_H

namespace Ice
{
    /**
     * Determines the order in which the Ice run time uses the endpoints in a proxy when establishing a connection.
     */
    enum class EndpointSelectionType : unsigned char
    {
        /**
         * <code>Random</code> causes the endpoints to be arranged in a random order.
         */
        Random,
        /**
         * <code>Ordered</code> forces the Ice run time to use the endpoints in the order they appeared in the proxy.
         */
        Ordered
    };
}

#endif
