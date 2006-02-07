// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    internal class Instance
    {
	internal Instance(Ice.Communicator communicator)
	{
	    facade_ = Ice.Util.getProtocolPluginFacade(communicator);
	    securityTraceLevel_ = communicator.getProperties().getPropertyAsIntWithDefault("IceSSL.Trace.Security", 0);
	    securityTraceCategory_ = "Security";
	    clientContext_ = new ClientContext(this);
	    serverContext_ = new ServerContext(this);
	    facade_.addEndpointFactory(new SslEndpointFactory(this));
	}

	internal Ice.Communicator communicator()
	{
	    return facade_.getCommunicator();
	}

	internal string defaultHost()
	{
	    return facade_.getDefaultHost();
	}

	internal int networkTraceLevel()
	{
	    return facade_.getNetworkTraceLevel();
	}

	internal string networkTraceCategory()
	{
	    return facade_.getNetworkTraceCategory();
	}

	internal int securityTraceLevel()
	{
	    return securityTraceLevel_;
	}

	internal string securityTraceCategory()
	{
	    return securityTraceCategory_;
	}

	internal ClientContext clientContext()
	{
	    return clientContext_;
	}

	internal ServerContext serverContext()
	{
	    return serverContext_;
	}

	private IceInternal.ProtocolPluginFacade facade_;
	private int securityTraceLevel_;
	private string securityTraceCategory_;
	private ClientContext clientContext_;
	private ServerContext serverContext_;
    }
}
