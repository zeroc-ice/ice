// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

final class TcpEndpointFactory implements EndpointFactory
{
    TcpEndpointFactory(ProtocolInstance instance)
    {
        _instance = instance;
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
        IPEndpointI endpt = new TcpEndpointI(_instance);
        endpt.initWithOptions(args, oaEndpoint);
        return endpt;
    }

    @Override
    public EndpointI read(com.zeroc.Ice.InputStream s)
    {
        return new TcpEndpointI(_instance, s);
    }

    @Override
    public void destroy()
    {
        _instance = null;
    }

    @Override
    public EndpointFactory clone(ProtocolInstance instance)
    {
        return new TcpEndpointFactory(instance);
    }

    private ProtocolInstance _instance;
}
