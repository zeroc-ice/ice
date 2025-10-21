// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Represents a batch request.
 * A batch request is created by invoking an operation on a batch-oneway or batch-datagram proxy.
 */
public interface BatchRequest {
    /** Queues this request. */
    void enqueue();

    /**
     * Returns the marshaled size of the request.
     *
     * @return the number of bytes consumed by the request
     */
    int getSize();

    /**
     * Returns the name of the operation.
     *
     * @return the operation name
     */
    String getOperation();

    /**
     * Returns the proxy used to create this batch request.
     *
     * @return the proxy
     */
    ObjectPrx getProxy();
}
