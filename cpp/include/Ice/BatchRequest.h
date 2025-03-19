// Copyright (c) ZeroC, Inc.

#ifndef ICE_BATCH_REQUEST_H
#define ICE_BATCH_REQUEST_H

#include "Config.h"

#include <string_view>

namespace Ice
{
    class ObjectPrx;

    /// Represents an invocation on a proxy configured for batch-oneway or batch-datagram.
    /// \headerfile Ice/Ice.h
    class ICE_API BatchRequest
    {
    public:
        virtual ~BatchRequest();

        /// Queues the request for an eventual flush.
        virtual void enqueue() const = 0;

        /// Obtains the size of the request.
        /// @return The number of bytes consumed by the request.
        [[nodiscard]] virtual int getSize() const = 0;

        /// Obtains the name of the operation.
        /// @return The operation name.
        [[nodiscard]] virtual std::string_view getOperation() const = 0;

        /// Obtains the proxy on which the batch request was invoked.
        /// @return The originating proxy.
        [[nodiscard]] virtual const ObjectPrx& getProxy() const = 0;
    };
}

#endif
