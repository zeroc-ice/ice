// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
        return Ice.TCPEndpointType.value;
    }

    public String
    protocol()
    {
        return "tcp";
    }

    public EndpointI
    create(String str, boolean oaEndpoint)
    {
        return new TcpEndpointI(_instance, str, oaEndpoint);
    }

    public EndpointI
    read(BasicStream s)
    {
        return new TcpEndpointI(s);
    }

    public void
    destroy()
    {
        _instance = null;
    }

    private Instance _instance;
}
