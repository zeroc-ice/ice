// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Represents an invocation on a proxy configured for batch-oneway or batch-datagram.
/// </summary>
public interface BatchRequest
{
    /// <summary>
    /// Queues the request for an eventual flush.
    /// </summary>
    void enqueue();

    /// <summary>
    /// Get the marshaled size of the request.
    /// </summary>
    /// <returns>The request size.</returns>
    int getSize();

    /// <summary>
    /// Gets the name of the operation.
    /// </summary>
    /// <returns>The request operation.</returns>
    string getOperation();

    /// <summary>
    /// The proxy used to invoke the batch request.
    /// </summary>
    /// <returns>The request proxy.</returns>
    ObjectPrx getProxy();
}
