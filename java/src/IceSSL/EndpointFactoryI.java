// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class EndpointFactoryI implements IceInternal.EndpointFactory
{
    EndpointFactoryI(Instance instance)
    {
        _instance = instance;
    }

    public short
    type()
    {
        return EndpointType.value;
    }

    public String
    protocol()
    {
        return "ssl";
    }

    public IceInternal.EndpointI
    create(String str, boolean oaEndpoint)
    {
        return new EndpointI(_instance, str, oaEndpoint);
    }

    public IceInternal.EndpointI
    read(IceInternal.BasicStream s)
    {
        return new EndpointI(_instance, s);
    }

    public void
    destroy()
    {
        _instance = null;
    }

    private Instance _instance;
}
