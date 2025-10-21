// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Base interface for listening to batch request queues. */
@FunctionalInterface
public interface BatchRequestInterceptor {
    /**
     * Called by the Ice runtime when a batch request is about to be added to the batch request
     * queue of a proxy or connection.
     *
     * <p>The implementation of this method must call {@code enqueue()} on the request to confirm its
     * addition to the queue; if it isn't called then the request isn't added to the queue.
     * The implementation can throw an Ice local exception to notify the caller of a failure.
     *
     * @param request the request to be added
     * @param queueBatchRequestCount the number of requests in the batch
     * @param queueBatchRequestSize the number of bytes in the batch
     */
    void enqueue(BatchRequest request, int queueBatchRequestCount, int queueBatchRequestSize);
}
