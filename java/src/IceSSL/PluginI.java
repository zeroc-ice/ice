// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

public class PluginI extends Ice.LocalObjectImpl implements Ice.Plugin
{
    public
    PluginI(Ice.Communicator communicator, String name, String[] args)
    {
	javax.net.ssl.SSLContext ctx = null;
	try
	{
	    ctx = javax.net.ssl.SSLContext.getInstance("SSL");
	    ctx.init(null, null, null);
	}
	catch(java.security.GeneralSecurityException ex)
	{
	    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
	    e.reason = "unable to initialize SSLContext";
	    e.initCause(ex);
	    throw e;
	}

	IceInternal.Instance instance = Ice.Util.getInstance(communicator);
	instance.endpointFactoryManager().add(new SslEndpointFactory(instance, ctx));
    }

    public void
    destroy()
    {
    }
}
