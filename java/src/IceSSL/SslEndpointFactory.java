// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class SslEndpointFactory implements IceInternal.EndpointFactory
{
    SslEndpointFactory(Instance instance)
    {
	_instance = instance;
    }

    public short
    type()
    {
	return SslEndpoint.TYPE;
    }

    public String
    protocol()
    {
	return "ssl";
    }

    public IceInternal.Endpoint
    create(String str)
    {
	return new SslEndpoint(_instance, str);
    }

    public IceInternal.Endpoint
    read(IceInternal.BasicStream s)
    {
	return new SslEndpoint(_instance, s);
    }

    public void
    destroy()
    {
	_instance = null;
    }

    private Instance _instance;
}
