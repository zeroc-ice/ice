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
    SslEndpointFactory(IceInternal.Instance instance, javax.net.ssl.SSLContext ctx)
    {
        _instance = instance;
	_ctx = ctx;
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
        return new SslEndpoint(_instance, _ctx, str);
    }

    public IceInternal.Endpoint
    read(IceInternal.BasicStream s)
    {
        return new SslEndpoint(s, _ctx);
    }

    public void
    destroy()
    {
        _instance = null;
    }

    private IceInternal.Instance _instance;
    private javax.net.ssl.SSLContext _ctx;
}
