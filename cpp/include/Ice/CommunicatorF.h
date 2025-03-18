// Copyright (c) ZeroC, Inc.

#ifndef ICE_COMMUNICATOR_F_H
#define ICE_COMMUNICATOR_F_H

#include <memory>

namespace Ice
{
    class Communicator;

    /// A shared pointer to a Communicator.
    using CommunicatorPtr = std::shared_ptr<Communicator>;
}

#endif
