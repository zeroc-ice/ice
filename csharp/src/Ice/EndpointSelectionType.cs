// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Determines how the Ice runtime sorts proxy endpoints when establishing a connection.
/// </summary>
public enum EndpointSelectionType
{
    /// <summary>
    /// The Ice runtime shuffles the endpoints in a random order.
    /// </summary>
    Random,

    /// <summary>
    /// The Ice runtime uses the endpoints in the order they appear in the proxy.
    /// </summary>
    Ordered
}
