// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.background;

final class EndpointFactory implements com.zeroc.IceInternal.EndpointFactory
{
    EndpointFactory(Configuration configuration, com.zeroc.IceInternal.EndpointFactory factory)
    {
        _configuration = configuration;
        _factory = factory;
    }

    @Override
    public short type()
    {
        return (short)(EndpointI.TYPE_BASE + _factory.type());
    }

    @Override
    public String protocol()
    {
        return "test-" + _factory.protocol();
    }

    @Override
    public com.zeroc.IceInternal.EndpointI create(java.util.ArrayList<String> args, boolean server)
    {
        return new EndpointI(_configuration, _factory.create(args, server));
    }

    @Override
    public com.zeroc.IceInternal.EndpointI read(com.zeroc.Ice.InputStream s)
    {
        short type = s.readShort();
        assert(type == _factory.type());

        s.startEncapsulation();
        com.zeroc.IceInternal.EndpointI endpoint = new EndpointI(_configuration, _factory.read(s));
        s.endEncapsulation();
        return endpoint;
    }

    @Override
    public void destroy()
    {
    }

    @Override
    public com.zeroc.IceInternal.EndpointFactory clone(com.zeroc.IceInternal.ProtocolInstance instance,
                                                       com.zeroc.IceInternal.EndpointFactory del)
    {
        return this;
    }

    private Configuration _configuration;
    private com.zeroc.IceInternal.EndpointFactory _factory;
}
