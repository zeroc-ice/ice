// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using System.Collections.Generic;
using System.Diagnostics;

internal class EndpointFactory : IceInternal.EndpointFactory
{
    internal EndpointFactory(IceInternal.EndpointFactory factory)
    {
        _factory = factory;
    }

    public void initialize()
    {
    }

    public short type()
    {
        return (short)(EndpointI.TYPE_BASE + _factory.type());
    }

    public string protocol()
    {
        return "test-" + _factory.protocol();
    }

    public IceInternal.EndpointI create(List<string> args, bool server)
    {
        return new EndpointI(_factory.create(args, server));
    }

    public IceInternal.EndpointI read(Ice.InputStream s)
    {
        short type = s.readShort();
        Debug.Assert(type == _factory.type());

        s.startEncapsulation();
        IceInternal.EndpointI endpoint = new EndpointI(_factory.read(s));
        s.endEncapsulation();
        return endpoint;
    }

    public void destroy()
    {
    }

    public IceInternal.EndpointFactory clone(IceInternal.ProtocolInstance instance)
    {
        return this;
    }

    private IceInternal.EndpointFactory _factory;
}
