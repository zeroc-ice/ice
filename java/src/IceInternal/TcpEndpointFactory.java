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

final class TcpEndpointFactory implements EndpointFactory
{
    TcpEndpointFactory(Instance instance)
    {
        _instance = instance;
    }

    public short
    type()
    {
        return TcpEndpoint.TYPE;
    }

    public String
    protocol()
    {
        return "tcp";
    }

    public Endpoint
    create(String str)
    {
        return new TcpEndpoint(_instance, str);
    }

    public Endpoint
    read(BasicStream s)
    {
        return new TcpEndpoint(s);
    }

    public void
    destroy()
    {
        _instance = null;
    }

    private Instance _instance;
}
