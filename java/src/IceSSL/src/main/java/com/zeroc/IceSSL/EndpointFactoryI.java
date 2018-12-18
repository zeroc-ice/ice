// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceSSL;

final class EndpointFactoryI extends com.zeroc.IceInternal.EndpointFactoryWithUnderlying
{
    EndpointFactoryI(Instance instance, short type)
    {
        super(instance, type);
        _instance = instance;
    }

    @Override
    public com.zeroc.IceInternal.EndpointFactory cloneWithUnderlying(com.zeroc.IceInternal.ProtocolInstance instance,
                                                                     short underlying)
    {
        return new EndpointFactoryI(new Instance(_instance.engine(), instance.type(), instance.protocol()), underlying);
    }

    @Override
    public com.zeroc.IceInternal.EndpointI createWithUnderlying(com.zeroc.IceInternal.EndpointI underlying,
                                                                java.util.ArrayList<String> args,
                                                                boolean oaEndpoint)
    {
        return new EndpointI(_instance, underlying);
    }

    @Override
    public com.zeroc.IceInternal.EndpointI readWithUnderlying(com.zeroc.IceInternal.EndpointI underlying,
                                                              com.zeroc.Ice.InputStream s)
    {
        return new EndpointI(_instance, underlying);
    }

    private Instance _instance;
}
