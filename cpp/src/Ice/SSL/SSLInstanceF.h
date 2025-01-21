// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_INSTANCE_F_H
#define ICE_SSL_INSTANCE_F_H

#include <memory>

namespace Ice::SSL
{
    class Instance;
    using InstancePtr = std::shared_ptr<Instance>;

    class EndpointI;
    using EndpointIPtr = std::shared_ptr<EndpointI>;

    class AcceptorI;
}

#endif
