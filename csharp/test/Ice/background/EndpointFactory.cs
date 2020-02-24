//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;

internal class EndpointFactory : IceInternal.IEndpointFactory
{
    internal EndpointFactory(IceInternal.IEndpointFactory factory)
    {
        _factory = factory;
    }

    public void Initialize()
    {
    }

    public short Type()
    {
        return (short)(Endpoint.TYPE_BASE + _factory.Type());
    }

    public string Transport()
    {
        return "test-" + _factory.Transport();
    }

    public IceInternal.Endpoint Create(List<string> args, bool server)
    {
        return new Endpoint(_factory.Create(args, server));
    }

    public IceInternal.Endpoint Read(Ice.InputStream s)
    {
        short type = s.ReadShort();
        Debug.Assert(type == _factory.Type());
        IceInternal.Endpoint endpoint = new Endpoint(_factory.Read(s));
        return endpoint;
    }

    public void Destroy()
    {
    }

    public IceInternal.IEndpointFactory Clone(IceInternal.TransportInstance instance)
    {
        return this;
    }

    private IceInternal.IEndpointFactory _factory;
}
