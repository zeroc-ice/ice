// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
        return UdpEndpointI.TYPE;
    }

    public String
    protocol()
    {
        return "udp";
    }

    public EndpointI
    create(String str)
    {
        return new UdpEndpointI(_instance, str);
    }

    public EndpointI
    read(BasicStream s)
    {
        return new UdpEndpointI(s);
    }

    public void
    destroy()
    {
        _instance = null;
    }

    private Instance _instance;
}
