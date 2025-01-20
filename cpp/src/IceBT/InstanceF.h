// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_INSTANCE_F_H
#define ICE_BT_INSTANCE_F_H

#include <memory>

namespace IceBT
{
    class Instance;
    using InstancePtr = std::shared_ptr<Instance>;

    class EndpointI;
    using EndpointIPtr = std::shared_ptr<EndpointI>;

    class TransceiverI;
    using TransceiverIPtr = std::shared_ptr<TransceiverI>;

    class AcceptorI;
    using AcceptorIPtr = std::shared_ptr<AcceptorI>;
}

#endif
