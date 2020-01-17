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

    public void initialize()
    {
    }

    public short type()
    {
        return (short)(Endpoint.TYPE_BASE + _factory.type());
    }

    public string protocol()
    {
        return "test-" + _factory.protocol();
    }

    public IceInternal.Endpoint create(List<string> args, bool server)
    {
        return new Endpoint(_factory.create(args, server));
    }

    public IceInternal.Endpoint read(Ice.InputStream s)
    {
        short type = s.ReadShort();
        Debug.Assert(type == _factory.type());
        IceInternal.Endpoint endpoint = new Endpoint(_factory.read(s));
        return endpoint;
    }

    public void destroy()
    {
    }

    public IceInternal.IEndpointFactory clone(IceInternal.ProtocolInstance instance)
    {
        return this;
    }

    private IceInternal.IEndpointFactory _factory;
}
