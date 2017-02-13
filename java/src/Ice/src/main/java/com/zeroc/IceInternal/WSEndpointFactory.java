// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

final public class WSEndpointFactory implements EndpointFactory
{
    public WSEndpointFactory(ProtocolInstance instance, EndpointFactory delegate)
    {
        _instance = instance;
        _delegate = delegate;
    }

    @Override
    public short type()
    {
        return _instance.type();
    }

    @Override
    public String protocol()
    {
        return _instance.protocol();
    }

    @Override
    public EndpointI create(java.util.ArrayList<String> args, boolean oaEndpoint)
    {
        return new WSEndpoint(_instance, _delegate.create(args, oaEndpoint), args);
    }

    @Override
    public EndpointI read(com.zeroc.Ice.InputStream s)
    {
        return new WSEndpoint(_instance, _delegate.read(s), s);
    }

    @Override
    public void destroy()
    {
        _delegate.destroy();
        _instance = null;
    }

    @Override
    public EndpointFactory clone(ProtocolInstance instance, EndpointFactory delegate)
    {
        return new WSEndpointFactory(instance, delegate);
    }

    private ProtocolInstance _instance;
    private EndpointFactory _delegate;
}
