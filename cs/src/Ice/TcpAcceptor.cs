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
	    if(_traceLevels.network >= 1)
	    {
		string s = "stopping to accept tcp connections at " + ToString();
		_logger.trace(_traceLevels.networkCat, s);
	    }
	    
	    Socket fd = _fd;
	    if(fd != null)
	    {
		_fd = null;
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
	    
	    return new TcpTransceiver(_instance, fd);
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
	    _instance = instance;
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
	    catch(System.Exception ex)
	    {
		_fd = null;
		throw ex;
	    }
	}
	
	~TcpAcceptor()
	{
	    Debug.Assert(_fd == null);
	}
	
	private Instance _instance;
	private TraceLevels _traceLevels;
	private Ice.Logger _logger;
	private Socket _fd;
	private int _backlog;
	private IPEndPoint _addr;
    }

}
