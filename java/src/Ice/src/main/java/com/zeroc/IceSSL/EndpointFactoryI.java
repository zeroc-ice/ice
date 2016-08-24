// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceSSL;

import com.zeroc.IceInternal.EndpointFactory;

final class EndpointFactoryI implements EndpointFactory
{
    EndpointFactoryI(Instance instance, EndpointFactory delegate)
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
    public com.zeroc.IceInternal.EndpointI create(java.util.ArrayList<String> args, boolean oaEndpoint)
    {
        return new EndpointI(_instance, _delegate.create(args, oaEndpoint));
    }

    @Override
    public com.zeroc.IceInternal.EndpointI read(com.zeroc.Ice.InputStream s)
    {
        return new EndpointI(_instance, _delegate.read(s));
    }

    @Override
    public void destroy()
    {
        _delegate.destroy();
        _instance = null;
    }

    @Override
    public EndpointFactory clone(com.zeroc.IceInternal.ProtocolInstance inst, EndpointFactory delegate)
    {
        Instance instance = new Instance(_instance.engine(), inst.type(), inst.protocol());
        return new EndpointFactoryI(instance, delegate != null ? delegate : _delegate.clone(instance, null));
    }

    private Instance _instance;
    private com.zeroc.IceInternal.EndpointFactory _delegate;
}
