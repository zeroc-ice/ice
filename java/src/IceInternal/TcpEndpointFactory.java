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
