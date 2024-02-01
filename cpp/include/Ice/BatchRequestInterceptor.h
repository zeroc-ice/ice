//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_BATCH_REQUEST_INTERCEPTOR_H
#define ICE_BATCH_REQUEST_INTERCEPTOR_H

#include <IceUtil/Shared.h>
#include <Ice/ProxyF.h>

namespace Ice
{

/**
 * Represents an invocation on a proxy configured for batch-oneway or batch-datagram.
 * \headerfile Ice/Ice.h
 */
class BatchRequest
{
public:

    virtual ~BatchRequest()
    {
    }

    /**
     * Queues the request for an eventual flush.
     */
    virtual void enqueue() const = 0;

    /**
     * Obtains the size of the request.
     * @return The number of bytes consumed by the request.
     */
    virtual int getSize() const = 0;

    /**
     * Obtains the name of the operation.
     * @return The operation name.
     */
    virtual const std::string& getOperation() const = 0;

    /**
     * Obtains the proxy on which the batch request was invoked.
     * @return The originating proxy.
     */
    virtual const Ice::ObjectPrxPtr& getProxy() const = 0;
};

}

#endif
