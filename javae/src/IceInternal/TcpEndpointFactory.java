// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
