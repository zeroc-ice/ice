// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
	return SslEndpointI.TYPE;
    }

    public String
    protocol()
    {
	return "ssl";
    }

    public IceInternal.EndpointI
    create(String str)
    {
	return new SslEndpointI(_instance, str);
    }

    public IceInternal.EndpointI
    read(IceInternal.BasicStream s)
    {
	return new SslEndpointI(_instance, s);
    }

    public void
    destroy()
    {
	_instance = null;
    }

    private Instance _instance;
}
