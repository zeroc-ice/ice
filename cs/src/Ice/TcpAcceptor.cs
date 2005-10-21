// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System;
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;

    class TcpAcceptor : Acceptor
    {
	public virtual Socket fd()
	{
	    return _fd;
	}
	
	public virtual void close()
	{
            Socket fd;
	    lock(this)
	    {
		fd = _fd;
		_fd = null;
	    }
	    if(fd != null)
	    {
		if(_traceLevels.network >= 1)
		{
		    string s = "stopping to accept tcp connections at " + ToString();
		    _logger.trace(_traceLevels.networkCat, s);
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
	    Network.doListen(_fd, _backlog);
	    
	    if(_traceLevels.network >= 1)
	    {
		string s = "accepting tcp connections at " + ToString();
		_logger.trace(_traceLevels.networkCat, s);
	    }
	}
	
	public virtual Transceiver accept(int timeout)
	{
	    Socket fd = Network.doAccept(_fd, timeout);
	    Network.setBlock(fd, false);

	    if(_traceLevels.network >= 1)
	    {
		string s = "accepted tcp connection\n" + Network.fdToString(fd);
		_logger.trace(_traceLevels.networkCat, s);
	    }
	    
	    return new TcpTransceiver(instance_, fd);
	}

	public virtual void connectToSelf()
	{
	    Socket fd = Network.createSocket(false);
	    Network.setBlock(fd, false);
	    Network.doConnect(fd, _addr, -1);
	    Network.closeSocket(fd);
	}

	public override string ToString()
	{
	    return Network.addrToString(_addr);
	}
	
	internal bool equivalent(string host, int port)
	{
	    EndPoint addr = Network.getAddress(host, port);
	    return addr.Equals(_addr);
	}
	
	internal virtual int effectivePort()
	{
	    return _addr.Port;
	}
	
	internal
	TcpAcceptor(Instance instance, string host, int port)
	{
	    instance_ = instance;
	    _traceLevels = instance.traceLevels();
	    _logger = instance.logger();
	    _backlog = 0;
	    
	    if(_backlog <= 0)
	    {
		_backlog = 5;
	    }
	    
	    try
	    {
		_fd = Network.createSocket(false);
		Network.setBlock(_fd, false);
		_addr = Network.getAddress(host, port);
		if(_traceLevels.network >= 2)
		{
		    string s = "attempting to bind to tcp socket " + ToString();
		    _logger.trace(_traceLevels.networkCat, s);
		}
		_addr = Network.doBind(_fd, _addr);
	    }
	    catch(System.Exception)
	    {
		_fd = null;
		throw;
	    }
	}
	
#if DEBUG
	~TcpAcceptor()
	{
	    lock(this)
	    {
		IceUtil.Assert.FinalizerAssert(_fd == null);
	    }
	}
#endif
	
	private Instance instance_;
	private TraceLevels _traceLevels;
	private Ice.Logger _logger;
	private Socket _fd;
	private int _backlog;
	private IPEndPoint _addr;
    }

}
