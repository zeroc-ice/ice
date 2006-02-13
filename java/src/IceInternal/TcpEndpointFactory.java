// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
        return TcpEndpointI.TYPE;
    }

    public String
    protocol()
    {
        return "tcp";
    }

    public EndpointI
    create(String str)
    {
        return new TcpEndpointI(_instance, str);
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
