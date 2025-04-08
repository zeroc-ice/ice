// Copyright (c) ZeroC, Inc.

#ifndef ICE_BATCH_REQUEST_H
#define ICE_BATCH_REQUEST_H

#include "Config.h"

#include <string_view>

namespace Ice
{
    class ObjectPrx;

    /// Represents a batch request. A batch request is created by invoking an operation on a batch-oneway or
    /// batch-datagram proxy.
    /// @headerfile Ice/Ice.h
    class ICE_API BatchRequest
    {
    public:
        virtual ~BatchRequest();

        /// Queues this request.
        virtual void enqueue() const = 0;

        /// Gets the size of the request.
        /// @return The number of bytes consumed by the request.
        [[nodiscard]] virtual int getSize() const = 0;

        /// Gets the name of the operation.
        /// @return The operation name.
        [[nodiscard]] virtual std::string_view getOperation() const = 0;

        /// Gets the proxy used to create this batch request.
        /// @return The proxy.
        [[nodiscard]] virtual const ObjectPrx& getProxy() const = 0;
    };
}

#endif
