// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class UdpEndpointFactory implements EndpointFactory
{
    UdpEndpointFactory(Instance instance)
    {
        _instance = instance;
    }

    public short
    type()
    {
        return UdpEndpoint.TYPE;
    }

    public String
    protocol()
    {
        return "udp";
    }

    public Endpoint
    create(String str)
    {
        return new UdpEndpoint(_instance, str);
    }

    public Endpoint
    read(BasicStream s)
    {
        return new UdpEndpoint(s);
    }

    public void
    destroy()
    {
        _instance = null;
    }

    private Instance _instance;
}
