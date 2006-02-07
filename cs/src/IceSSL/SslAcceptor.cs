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
    using System;
    using System.Diagnostics;
    using System.Net;
    using System.Net.Security;
    using System.Net.Sockets;
    using System.Security.Cryptography.X509Certificates;

    class SslAcceptor : IceInternal.Acceptor
    {
	public virtual Socket fd()
	{
	    return fd_;
	}
	
	public virtual void close()
	{
            Socket fd;
	    lock(this)
	    {
		fd = fd_;
		fd_ = null;
	    }
	    if(fd != null)
	    {
		if(instance_.networkTraceLevel() >= 1)
		{
		    string s = "stopping to accept ssl connections at " + ToString();
		    logger_.trace(instance_.networkTraceCategory(), s);
		}
	    
		try
		{
		    fd.Close();
		}
		catch(System.Exception)
		{
		    // Ignore.
		}
	    }
	}
	
	public virtual void listen()
	{
	    IceInternal.Network.doListen(fd_, backlog_);
	    
	    if(instance_.networkTraceLevel() >= 1)
	    {
		string s = "accepting ssl connections at " + ToString();
		logger_.trace(instance_.networkTraceCategory(), s);
	    }
	}
	
	public virtual IceInternal.Transceiver accept(int timeout)
	{
	    Debug.Assert(timeout == -1); // Always called with -1 for thread-per-connection.

	    Socket fd = IceInternal.Network.doAccept(fd_, timeout);
	    IceInternal.Network.setBlock(fd, true); // SSL requires a blocking socket.

	    SslStream stream = null;
	    try
	    {
		stream = instance_.serverContext().authenticate(fd, timeout);
	    }
	    catch(Ice.ConnectionLostException)
	    {
		//
		// This situation occurs when connectToSelf is called; the "remote" end
		// closes the socket immediately.
		//
		return null; // TODO: Correct?
	    }

	    if(instance_.networkTraceLevel() >= 1)
	    {
		string s = "accepted ssl connection\n" + IceInternal.Network.fdToString(fd);
		logger_.trace(instance_.networkTraceCategory(), s);
	    }
	    
	    return new SslTransceiver(instance_, fd, stream);
	}

	public virtual void connectToSelf()
	{
	    Socket fd = IceInternal.Network.createSocket(false);
	    IceInternal.Network.setBlock(fd, false);
	    IceInternal.Network.doConnect(fd, addr_, -1);
	    IceInternal.Network.closeSocket(fd);
	}

	public override string ToString()
	{
	    return IceInternal.Network.addrToString(addr_);
	}
	
	internal bool equivalent(string host, int port)
	{
	    EndPoint addr = IceInternal.Network.getAddress(host, port);
	    return addr.Equals(addr_);
	}
	
	internal virtual int effectivePort()
	{
	    return addr_.Port;
	}
	
	internal
	SslAcceptor(Instance instance, string host, int port)
	{
	    instance_ = instance;
	    logger_ = instance.communicator().getLogger();
	    backlog_ = 0;
	    
	    if(backlog_ <= 0)
	    {
		backlog_ = 5;
	    }
	    
	    try
	    {
		fd_ = IceInternal.Network.createSocket(false);
		IceInternal.Network.setBlock(fd_, false);
		addr_ = IceInternal.Network.getAddress(host, port);
		if(instance_.networkTraceLevel() >= 2)
		{
		    string s = "attempting to bind to ssl socket " + ToString();
		    logger_.trace(instance_.networkTraceCategory(), s);
		}
		addr_ = IceInternal.Network.doBind(fd_, addr_);
	    }
	    catch(System.Exception)
	    {
		fd_ = null;
		throw;
	    }
	}
	
#if DEBUG
	~SslAcceptor()
	{
	    lock(this)
	    {
		IceUtil.Assert.FinalizerAssert(fd_ == null);
	    }
	}
#endif
	
	private Instance instance_;
	private Ice.Logger logger_;
	private Socket fd_;
	private int backlog_;
	private IPEndPoint addr_;
    }
}
