//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Represents an invocation on a proxy configured for batch-oneway or batch-datagram.
 **/
public interface BatchRequest
{
    /**
     * Confirms the queuing of the batch request.
     **/
    void enqueue();

    /**
     * The marshaled size of the request.
     * @return The marshaled size.
     **/
    int getSize();

    /**
     * The name of the operation.
     * @return The operation name.
     **/
    String getOperation();

    /**
     * The proxy used to invoke the batch request.
     * @return The proxy.
     **/
    ObjectPrx getProxy();
}
