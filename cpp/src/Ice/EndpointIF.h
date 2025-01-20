// Copyright (c) ZeroC, Inc.

#ifndef ICE_ENDPOINT_I_F_H
#define ICE_ENDPOINT_I_F_H

#include <memory>

namespace IceInternal
{
    class EndpointI;
    class TcpEndpointI;
    class UdpEndpointI;
    class WSEndpoint;

    using EndpointIPtr = std::shared_ptr<EndpointI>;
    using TcpEndpointIPtr = std::shared_ptr<TcpEndpointI>;
    using UdpEndpointIPtr = std::shared_ptr<UdpEndpointI>;
    using WSEndpointPtr = std::shared_ptr<WSEndpoint>;
}

#endif
