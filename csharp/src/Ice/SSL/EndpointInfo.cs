// Copyright (c) ZeroC, Inc.

namespace Ice.SSL;

public abstract class EndpointInfo : Ice.EndpointInfo
{
    protected EndpointInfo()
    {
    }

    protected EndpointInfo(Ice.EndpointInfo underlying, int timeout, bool compress)
        : base(underlying, timeout, compress)
    {
    }
}
