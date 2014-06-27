// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceWS;

final class EndpointFactoryI implements IceInternal.EndpointFactory
{
    EndpointFactoryI(Instance instance, IceInternal.EndpointFactory delegate)
    {
        _instance = instance;
        _delegate = delegate;
    }

    public short type()
    {
        return _instance.type();
    }

    public String protocol()
    {
        return _instance.protocol();
    }

    public IceInternal.EndpointI create(java.util.ArrayList<String> args, boolean oaEndpoint)
    {
        return new EndpointI(_instance, _delegate.create(args, oaEndpoint), args);
    }

    public IceInternal.EndpointI read(IceInternal.BasicStream s)
    {
        return new EndpointI(_instance, _delegate.read(s), s);
    }

    public void destroy()
    {
        _delegate.destroy();
        _instance = null;
    }

    public IceInternal.EndpointFactory clone(IceInternal.ProtocolInstance instance)
    {
        assert(false); // We don't support cloning this transport.
        return null;
    }

    private Instance _instance;
    private IceInternal.EndpointFactory _delegate;
}
