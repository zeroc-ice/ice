// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
