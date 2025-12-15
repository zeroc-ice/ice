// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Represents a batch request. A batch request is created by invoking an operation on a batch-oneway or
/// batch-datagram proxy.
/// </summary>
public interface BatchRequest
{
    /// <summary>
    /// Queues this request.
    /// </summary>
    void enqueue();

    /// <summary>
    /// Gets the size of the request.
    /// </summary>
    /// <returns>The number of bytes consumed by the request.</returns>
    int getSize();

    /// <summary>
    /// Gets the name of the operation.
    /// </summary>
    /// <returns>The operation name.</returns>
    string getOperation();

    /// <summary>
    /// Gets the proxy used to create this batch request.
    /// </summary>
    /// <returns>The proxy.</returns>
    ObjectPrx getProxy();
}
