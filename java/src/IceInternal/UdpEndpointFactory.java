// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class UdpEndpointFactory implements EndpointFactory
{
    UdpEndpointFactory(ProtocolInstance instance)
    {
        _instance = instance;
    }

    public short type()
    {
        return _instance.type();
    }

    public String protocol()
    {
        return _instance.protocol();
    }

    public EndpointI create(java.util.ArrayList<String> args, boolean oaEndpoint)
    {
        IPEndpointI endpt = new UdpEndpointI(_instance);
        endpt.initWithOptions(args, oaEndpoint);
        return endpt;
    }

    public EndpointI read(BasicStream s)
    {
        return new UdpEndpointI(_instance, s);
    }

    public void destroy()
    {
        _instance = null;
    }

    public EndpointFactory clone(ProtocolInstance instance)
    {
        return new UdpEndpointFactory(instance);
    }

    private ProtocolInstance _instance;
}
