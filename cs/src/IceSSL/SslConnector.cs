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
    using System.Net;
    using System.Net.Security;
    using System.Net.Sockets;

    sealed class SslConnector : IceInternal.Connector
    {
	public IceInternal.Transceiver connect(int timeout)
	{
	    if(instance_.networkTraceLevel() >= 2)
	    {
		string s = "trying to establish ssl connection to " + ToString();
		logger_.trace(instance_.networkTraceCategory(), s);
	    }

	    Socket fd = IceInternal.Network.createSocket(false);
	    IceInternal.Network.setBlock(fd, true);
	    IceInternal.Network.doConnectAsync(fd, addr_, timeout);

	    // TODO: Catch exceptions?
	    SslStream stream = instance_.clientContext().authenticate(fd, host_, timeout);

	    if(instance_.networkTraceLevel() >= 1)
	    {
		string s = "ssl connection established\n" + IceInternal.Network.fdToString(fd);
		logger_.trace(instance_.networkTraceCategory(), s);
	    }

	    return new SslTransceiver(instance_, fd, stream);
	}

	public override string ToString()
	{
	    return IceInternal.Network.addrToString(addr_);
	}

	//
	// Only for use by SslEndpoint.
	//
	internal SslConnector(Instance instance, string host, int port)
	{
	    instance_ = instance;
	    host_ = host;
	    logger_ = instance.communicator().getLogger();
	    addr_ = IceInternal.Network.getAddress(host, port);
	}

	private Instance instance_;
	private string host_;
	private Ice.Logger logger_;
	private IPEndPoint addr_;
    }
}
