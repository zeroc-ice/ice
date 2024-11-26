// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.SSL;

public sealed class EndpointInfo : Ice.EndpointInfo
{
    internal EndpointInfo(Ice.EndpointInfo underlying)
        : base(underlying)
    {
    }
}
