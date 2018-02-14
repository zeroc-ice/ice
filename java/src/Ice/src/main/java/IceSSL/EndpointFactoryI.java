// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class EndpointFactoryI implements IceInternal.EndpointFactory
{
    EndpointFactoryI(Instance instance)
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
    public IceInternal.EndpointI create(java.util.ArrayList<String> args, boolean oaEndpoint)
    {
        IceInternal.IPEndpointI endpt = new EndpointI(_instance);
        endpt.initWithOptions(args, oaEndpoint);
        return endpt;
    }

    @Override
    public IceInternal.EndpointI read(IceInternal.BasicStream s)
    {
        return new EndpointI(_instance, s);
    }

    @Override
    public void destroy()
    {
        _instance = null;
    }

    @Override
    public IceInternal.EndpointFactory clone(IceInternal.ProtocolInstance instance)
    {
        return new EndpointFactoryI(new Instance(_instance.engine(), instance.type(), instance.protocol()));
    }

    private Instance _instance;
}
