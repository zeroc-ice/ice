// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
    public Socket
    fd()
    {
	Debug.Assert(_fd != null);
	return _fd;
    }
    
    public void
    close()
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
	}
	catch(System.IO.IOException)
	{
	}
	_fd = null;
    }
    
    public void
    shutdown()
    {
	if(_traceLevels.network >= 2)
	{
	    string s = "shutting down tcp connection\n" + ToString();
	    _logger.trace(_traceLevels.networkCat, s);
	}
	
	Debug.Assert(_fd != null);
	try
	{
	    _fd.Shutdown(SocketShutdown.Send); // Shutdown socket for writing
	}
	catch(System.IO.IOException)
	{
	}
    }
    
    public void
    write(BasicStream stream, int timeout)
    {
	ByteBuffer buf = stream.prepareWrite();
	
	byte[] bytes = buf.toArray();
	int remaining = bytes.Length;
	try
	{
	    while(remaining > 0)
	    {   
		int ret;

		Debug.Assert(_fd != null);
		try
		{
		    ret = _fd.Send(bytes);
		}
		catch(Win32Exception e)
		{
		    if(Network.wouldBlock(e))
		    {
			ret = 0;
		    }
		    else
		    {
			throw;
		    }
		}
    	    
		if(ret == 0)
		{
		    if(timeout == 0)
		    {
			throw new Ice.TimeoutException();
		    }
		    ArrayList sendList = new ArrayList();
		    sendList.Add(_fd);
		    ArrayList errorList = new ArrayList();
		    errorList.Add(_fd);
		    Network.doSelect(null, sendList, errorList, timeout);
		    if(errorList.Count != 0)
		    {
			 throw new Ice.ConnectionLostException();
		    }
		    if(sendList.Count == 0)
		    {
			if(timeout > 0)
			{
			    throw new Ice.TimeoutException();
			}
			else
			{
			    throw new Ice.ConnectionLostException();
			}
		    }
		}
    	    
		if(_traceLevels.network >= 3)
		{
		    string s = "sent " + ret + " of " + remaining + " bytes via tcp\n" + ToString();
		    _logger.trace(_traceLevels.networkCat, s);
		}
    	    
		if(_stats != null)
		{
		    _stats.bytesSent("tcp", ret);
		}

		remaining -= ret;	
	    }
	}
	catch(Ice.Exception)
	{
	    throw;
	}
	catch(SocketException ex)
	{
	    if(Network.connectionLost(ex))
	    {
		throw new Ice.ConnectionLostException(ex);
	    }
	    throw new Ice.SocketException(ex);
	}
	catch(System.Exception ex)
	{
	    throw new Ice.SyscallException(ex);
	}
    }
    
    public void
    read(BasicStream stream, int timeout)
    {
	ByteBuffer buf = stream.prepareRead();
	
	int remaining = buf.remaining();
	
	try
	{
	    ArrayList readList = new ArrayList();
	    readList.Add(_fd);
	    Network.doSelect(readList, null, null, timeout);

	    while(remaining > 0)
	    {
		Debug.Assert(_fd != null);

		int ret;
		byte[] bytes = new byte[remaining];
		int available = _fd.Available;
		if(available == 0)
		{
		    if(timeout == 0)
		    {
			throw new Ice.TimeoutException();
		    }
		    ArrayList recvList = new ArrayList();
		    recvList.Add(_fd);
		    ArrayList errorList = new ArrayList();
		    errorList.Add(_fd);
		    Network.doSelect(recvList, null, errorList, timeout);
		    if(errorList.Count != 0)
		    {
			throw new Ice.ConnectionLostException();
		    }
		    if(recvList.Count == 0)
		    {
			if(timeout >= 0)
			{
			    throw new Ice.TimeoutException();
			}
			else
			{
			    throw new Ice.ConnectionLostException();
			}
		    }	
		}
		ret = _fd.Receive(bytes);
		
		if(_traceLevels.network >= 3)
		{
		    string s = "received " + ret + " of " + remaining + " bytes via tcp\n" + ToString();
		    _logger.trace(_traceLevels.networkCat, s);
		}
		
		if(_stats != null)
		{
		    _stats.bytesReceived("tcp", ret);
		}

		buf.put(bytes, buf.position(), ret);
		remaining -= ret;
	    }
	}
	catch(Ice.Exception)
	{
	    throw;
	}
	catch(SocketException ex)
	{
	    if(Network.connectionLost(ex))
	    {
		throw new Ice.ConnectionLostException(ex);
	    }
	    throw new Ice.SocketException(ex);
	}
	catch(System.Exception ex)
	{
	    throw new Ice.SyscallException(ex);
	}
    }
    
    public override string
    ToString()
    {
	return _desc;
    }
    
    //
    // Only for use by TcpConnector, TcpAcceptor
    //
    internal
    TcpTransceiver(Instance instance, Socket fd)
    {
	_fd = fd;
	_traceLevels = instance.traceLevels();
	_logger = instance.logger();
	_stats = instance.stats();
	_desc = Network.fdToString(_fd);
    }
    
    ~TcpTransceiver()
    {
	Debug.Assert(_fd == null);
    }
    
    private Socket _fd;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private Ice.Stats _stats;
    private string _desc;
}

}
