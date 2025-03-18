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

    /// A shared pointer to an Endpoint.
    using EndpointPtr = std::shared_ptr<Endpoint>;

    /// A shared pointer to an EndpointInfo.
    using EndpointInfoPtr = std::shared_ptr<EndpointInfo>;

    /// A shared pointer to an IPEndpointInfo.
    using IPEndpointInfoPtr = std::shared_ptr<IPEndpointInfo>;

    /// A shared pointer to an OpaqueEndpointInfo.
    using OpaqueEndpointInfoPtr = std::shared_ptr<OpaqueEndpointInfo>;

    /// A shared pointer to a TCPEndpointInfo.
    using TCPEndpointInfoPtr = std::shared_ptr<TCPEndpointInfo>;

    /// A shared pointer to a UDPEndpointInfo.
    using UDPEndpointInfoPtr = std::shared_ptr<UDPEndpointInfo>;

    /// A shared pointer to a WSEndpointInfo.
    using WSEndpointInfoPtr = std::shared_ptr<WSEndpointInfo>;

    /// A sequence of endpoints.
    using EndpointSeq = std::vector<EndpointPtr>;
}

#endif
