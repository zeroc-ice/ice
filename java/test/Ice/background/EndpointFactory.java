// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package test.Ice.background;

final class EndpointFactory implements IceInternal.EndpointFactory
{
    EndpointFactory(Configuration configuration, IceInternal.EndpointFactory factory)
    {
        _configuration = configuration;
        _factory = factory;
    }

    public short
    type()
    {
        return (short)(EndpointI.TYPE_BASE + _factory.type());
    }

    public String
    protocol()
    {
        return "test-" + _factory.protocol();
    }

    public IceInternal.EndpointI
    create(String str, boolean server)
    {
        return new EndpointI(_configuration, _factory.create(str, server));
    }

    public IceInternal.EndpointI
    read(IceInternal.BasicStream s)
    {
        s.readShort();
        return new EndpointI(_configuration, _factory.read(s));
    }

    public void
    destroy()
    {
    }

    private Configuration _configuration;
    private IceInternal.EndpointFactory _factory;
}
