// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
