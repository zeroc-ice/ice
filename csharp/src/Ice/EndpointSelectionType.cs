// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Determines the order in which the Ice run time uses the endpoints in a proxy when establishing a connection.
/// </summary>
public enum EndpointSelectionType
{
    /// <summary>
    /// Random causes the endpoints to be arranged in a random order.
    /// </summary>
    Random,

    /// <summary>
    /// Ordered forces the Ice run time to use the endpoints in the order they appeared in the proxy.
    /// </summary>
    Ordered
}
