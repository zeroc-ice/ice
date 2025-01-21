// Copyright (c) ZeroC, Inc.

#ifndef ICE_IP_ENDPOINT_I_F_H
#define ICE_IP_ENDPOINT_I_F_H

#include <memory>

namespace IceInternal
{
    class IPEndpointI;
    using IPEndpointIPtr = std::shared_ptr<IPEndpointI>;

    class EndpointHostResolver;
    using EndpointHostResolverPtr = std::shared_ptr<EndpointHostResolver>;
}

#endif
