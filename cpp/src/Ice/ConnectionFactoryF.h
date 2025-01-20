// Copyright (c) ZeroC, Inc.

#ifndef ICE_CONNECTION_FACTORY_F_H
#define ICE_CONNECTION_FACTORY_F_H

#include <memory>

namespace IceInternal
{
    class OutgoingConnectionFactory;
    using OutgoingConnectionFactoryPtr = std::shared_ptr<OutgoingConnectionFactory>;

    class IncomingConnectionFactory;
    using IncomingConnectionFactoryPtr = std::shared_ptr<IncomingConnectionFactory>;
}

#endif
