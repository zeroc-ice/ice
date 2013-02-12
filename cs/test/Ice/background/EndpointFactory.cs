// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

    public IceInternal.EndpointI create(string str, bool server)
    {
        return new EndpointI(_factory.create(str, server));
    }

    public IceInternal.EndpointI read(IceInternal.BasicStream s)
    {
        s.readShort();
        return new EndpointI(_factory.read(s));
    }

    public void destroy()
    {
    }

    private IceInternal.EndpointFactory _factory;
}
