// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Represents an invocation on a proxy configured for batch-oneway or batch-datagram. */
public interface BatchRequest {
    /** Confirms the queuing of the batch request. */
    void enqueue();

    /**
     * The marshaled size of the request.
     *
     * @return the marshaled size
     */
    int getSize();

    /**
     * The name of the operation.
     *
     * @return the operation name
     */
    String getOperation();

    /**
     * The proxy used to invoke the batch request.
     *
     * @return the proxy
     */
    ObjectPrx getProxy();
}
