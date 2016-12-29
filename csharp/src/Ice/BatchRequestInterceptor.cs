// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    public interface BatchRequest
    {
        /// <summary>
        /// Confirms the queuing of the batch request.
        /// </summary>
        void enqueue();

        /// <summary>
        /// Get the marshalled size of the request.
        /// </summary>
        /// <returns>The request size.</returns>
        int getSize();

        /// <summary>
        /// Get the name of the operation
        /// </summary>
        /// <returns>The request operation.</returns>
        string getOperation();

        /// <summary>
        /// The proxy used to invoke the batch request.
        /// </summary>
        /// <returns>The request proxy.</returns>
        ObjectPrx getProxy();
    }

    /// <summary>
    /// Base interface for listening to batch request queues.
    /// </summary>
    public interface BatchRequestInterceptor
    {
        /// <summary>
        /// Called by the Ice runtime when a batch request is about to be
        /// added to the batch request queue of a proxy or connection.
        ///
        /// The implementation of this method must call enqueue() on the
        /// request to confirm its addition to the queue, if not called
        /// the request isn't added to the queue. The implementation can
        /// raise an Ice local exception to notify the caller of a failure.
        /// </summary>
        /// <param name="request">The batch request.</param>
        /// <param name="queueBatchRequestCount">The number of batch request queued.</param>
        /// <param name="queueBatchRequestSize">The size of the queued batch requests.</param>
        void enqueue(BatchRequest request, int queueBatchRequestCount, int queueBatchRequestSize);
    }
}
