// Copyright (c) ZeroC, Inc.

#ifndef ICE_ENDPOINT_F_H
#define ICE_ENDPOINT_F_H

#include <memory>
#include <vector>

namespace Ice
{
    class Endpoint;
    class EndpointInfo;
    class IPEndpointInfo;
    class OpaqueEndpointInfo;
    class TCPEndpointInfo;
    class UDPEndpointInfo;
    class WSEndpointInfo;

    using EndpointPtr = std::shared_ptr<Endpoint>;
    using EndpointInfoPtr = std::shared_ptr<EndpointInfo>;
    using IPEndpointInfoPtr = std::shared_ptr<IPEndpointInfo>;
    using OpaqueEndpointInfoPtr = std::shared_ptr<OpaqueEndpointInfo>;
    using TCPEndpointInfoPtr = std::shared_ptr<TCPEndpointInfo>;
    using UDPEndpointInfoPtr = std::shared_ptr<UDPEndpointInfo>;
    using WSEndpointInfoPtr = std::shared_ptr<WSEndpointInfo>;

    /**
     * A sequence of endpoints.
     */
    using EndpointSeq = std::vector<EndpointPtr>;
}

#endif
