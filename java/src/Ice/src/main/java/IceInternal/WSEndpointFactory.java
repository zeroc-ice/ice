// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final public class WSEndpointFactory implements IceInternal.EndpointFactory
{
    public WSEndpointFactory(ProtocolInstance instance, IceInternal.EndpointFactory delegate)
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
    public IceInternal.EndpointI create(java.util.ArrayList<String> args, boolean oaEndpoint)
    {
        return new WSEndpoint(_instance, _delegate.create(args, oaEndpoint), args);
    }

    @Override
    public IceInternal.EndpointI read(IceInternal.BasicStream s)
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
    public IceInternal.EndpointFactory clone(IceInternal.ProtocolInstance instance)
    {
        assert(false); // We don't support cloning this transport.
        return null;
    }

    private ProtocolInstance _instance;
    private IceInternal.EndpointFactory _delegate;
}
