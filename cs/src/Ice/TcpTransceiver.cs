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

    using System.Collections;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Net.Sockets;

    sealed class TcpTransceiver : Transceiver
    {
	public Socket fd()
	{
	    Debug.Assert(_fd != null);
	    return _fd;
	}
	
	public void close()
	{
	    if(_traceLevels.network >= 1)
	    {
		string s = "closing tcp connection\n" + ToString();
		_logger.trace(_traceLevels.networkCat, s);
	    }
	    
	    Debug.Assert(_fd != null);
	    try
	    {
		_fd.Close();
		_fd = null;
	    }
	    catch(System.IO.IOException ex)
	    {
	        throw new Ice.SocketException(ex);
	    }
	}

	public void shutdownWrite()
	{
	    if(_traceLevels.network >= 2)
	    {
		string s = "shutting down tcp connection for writing\n" + ToString();
		_logger.trace(_traceLevels.networkCat, s);
	    }
	    
	    Debug.Assert(_fd != null);
	    try
	    {
		_fd.Shutdown(SocketShutdown.Send);
	    }
	    catch(SocketException ex)
	    {
	        if(Network.notConnected(ex))
		{
		    return;
		}
	        throw new Ice.SocketException(ex);
	    }
	}

	public void shutdownReadWrite()
	{
	    if(_traceLevels.network >= 2)
	    {
		string s = "shutting down tcp connection for reading and writing\n" + ToString();
		_logger.trace(_traceLevels.networkCat, s);
	    }
	    
	    Debug.Assert(_fd != null);
	    try
	    {
		_fd.Shutdown(SocketShutdown.Both);
	    }
	    catch(SocketException ex)
	    {
	        if(Network.notConnected(ex))
		{
		    return;
		}
	        throw new Ice.SocketException(ex);
	    }
	}
	
	public void write(BasicStream stream, int timeout)
	{
	    Debug.Assert(_fd != null);

	    ByteBuffer buf = stream.prepareWrite();
	    int remaining = buf.remaining();
	    int position = buf.position();
	    try
	    {
		while(remaining > 0)
		{   
		    int ret;
		    try
		    {
			//
			// Try to send first. Most of the time, this will work and
			// avoids the cost of calling Poll().
			//
			ret = _fd.Send(buf.rawBytes(), position, remaining, SocketFlags.None);
			Debug.Assert(ret != 0);
		    }
		    catch(Win32Exception e)
		    {
			if(Network.wouldBlock(e))
			{
			    if(timeout == 0)
			    {
				throw new Ice.TimeoutException();
			    }
			    ret = 0;
			}
			else
			{
			    throw;
			}
		    }
		    if(ret == 0)
		    {
			//
			// The first attempt to write would have blocked,
			// so wait for the socket to become writable now.
			//
			if(!Network.doPoll(_fd, timeout, Network.PollMode.Write))
			{
			    throw new Ice.TimeoutException();
			}
			ret = _fd.Send(buf.rawBytes(), position, remaining, SocketFlags.None);
			Debug.Assert(ret != 0);
		    }

		    if(_traceLevels.network >= 3)
		    {
			string s = "sent " + ret + " of " + remaining + " bytes via tcp\n" + ToString();
			_logger.trace(_traceLevels.networkCat, s);
		    }
		    if(_stats != null)
		    {
			_stats.bytesSent(type(), ret);
		    }

		    remaining -= ret;
		    buf.position(position += ret);
		}
	    }
	    catch(SocketException ex)
	    {
		if(Network.connectionLost(ex))
		{
		    throw new Ice.ConnectionLostException(ex);
		}
		if(Network.wouldBlock(ex))
		{
		    throw new Ice.TimeoutException();
		}
		throw new Ice.SocketException(ex);
	    }
	    catch(Ice.LocalException)
	    {
		throw;
	    }
	    catch(System.Exception ex)
	    {
		throw new Ice.SyscallException(ex);
	    }
	}
	
	public void read(BasicStream stream, int timeout)
	{
	    Debug.Assert(_fd != null);

	    ByteBuffer buf = stream.prepareRead();    
	    int remaining = buf.remaining();
	    int position = buf.position();
	    
	    try
	    {
		while(remaining > 0)
		{
		    int ret;
		    try
		    {
			//
			// Try to receive first. Much of the time, this will work and we
			// avoid the cost of calling Poll().
			//	
			ret = _fd.Receive(buf.rawBytes(), position, remaining, SocketFlags.None);
		    }
		    catch(Win32Exception e)
		    {
			if(Network.wouldBlock(e))
			{
			    if(timeout == 0)
			    {
				throw new Ice.TimeoutException();
			    }
			    ret = 0;
			}
			else
			{
			    throw;
			}
		    }
		    if(ret == 0)
		    {
			if(!Network.doPoll(_fd, timeout, Network.PollMode.Read))
			{
			    throw new Ice.TimeoutException();
			}
			ret = _fd.Receive(buf.rawBytes(), position, remaining, SocketFlags.None);
			if(ret == 0)
			{
			    throw new Ice.ConnectionLostException();
			}
		    }  		    
		    if(_traceLevels.network >= 3)
		    {
			string s = "received " + ret + " of " + remaining + " bytes via tcp\n" + ToString();
			_logger.trace(_traceLevels.networkCat, s);
		    }    
		    if(_stats != null)
		    {
			_stats.bytesReceived(type(), ret);
		    }
		    remaining -= ret;
		    buf.position(position += ret);
		}
	    }
	    catch(SocketException ex)
	    {
		if(Network.connectionLost(ex))
		{
		    throw new Ice.ConnectionLostException(ex);
		}
		if(Network.wouldBlock(ex))
		{
		    throw new Ice.TimeoutException();
		}
		throw new Ice.SocketException(ex);
	    }
	    catch(Ice.LocalException)
	    {
		throw;
	    }
	    catch(System.Exception ex)
	    {
		throw new Ice.SyscallException(ex);
	    }
	}

	public string type()
	{
	    return "tcp";
	}

	public override string ToString()
	{
	    return _desc;
	}
	
	//
	// Only for use by TcpConnector, TcpAcceptor
	//
	internal TcpTransceiver(Instance instance, Socket fd)
	{
	    _fd = fd;
	    _traceLevels = instance.traceLevels();
	    _logger = instance.logger();
	    _stats = instance.stats();
	    _desc = Network.fdToString(_fd);
	}
	
	~TcpTransceiver()
	{
#if DEBUG
	    lock(this)
	    {
		Debug.Assert(_fd == null);
	    }
#endif DEBUG
	}
	
	private Socket _fd;
	private TraceLevels _traceLevels;
	private Ice.Logger _logger;
	private Ice.Stats _stats;
	private string _desc;
    }

}
