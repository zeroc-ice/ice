// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.SSL;

/// <summary>
/// Provides access to an SSL endpoint's information.
/// </summary>
public sealed class EndpointInfo : Ice.EndpointInfo
{
    internal EndpointInfo(Ice.EndpointInfo underlying)
        : base(underlying)
    {
    }
}
