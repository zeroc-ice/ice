// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    public IceInternal.EndpointI read(IceInternal.BasicStream s)
    {
        short type = s.readShort();
        Debug.Assert(type == _factory.type());

        s.startReadEncaps();
        IceInternal.EndpointI endpoint = new EndpointI(_factory.read(s));
        s.endReadEncaps();
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
