//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ENDPOINT_F_H
#define ICE_ENDPOINT_F_H

#include <memory>
#include <vector>

namespace Ice
{

    class EndpointInfo;
    using EndpointInfoPtr = std::shared_ptr<EndpointInfo>;

    class IPEndpointInfo;
    using IPEndpointInfoPtr = std::shared_ptr<IPEndpointInfo>;

    class TCPEndpointInfo;
    using TCPEndpointInfoPtr = std::shared_ptr<TCPEndpointInfo>;

    class UDPEndpointInfo;
    using UDPEndpointInfoPtr = std::shared_ptr<UDPEndpointInfo>;

    class WSEndpointInfo;
    using WSEndpointInfoPtr = std::shared_ptr<WSEndpointInfo>;

    class Endpoint;
    using EndpointPtr = std::shared_ptr<Endpoint>;

    /**
     * A sequence of endpoints.
     */
    using EndpointSeq = std::vector<EndpointPtr>;

}

#endif
