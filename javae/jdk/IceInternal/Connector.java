// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class Connector
{
    private static class ConnectThread extends Thread
    {
	ConnectThread(InetSocketAddress addr)
	{
	    _addr = addr;
	}

	public void
	run()
	{
	    try
	    {
		java.net.Socket fd = new java.net.Socket(_addr.getAddress(), _addr.getPort());
		synchronized(this)
		{
		    _fd = fd;
		    notifyAll();
		}
	    }
	    catch(java.io.IOException ex)
	    {
		synchronized(this)
		{
		    _ex = ex;
		    notifyAll();
		}
	    }
	}

	java.net.Socket
	getFd(int timeout)
	    throws java.io.IOException
	{
	    java.net.Socket fd = null;

	    synchronized(this)
	    {
		while(_fd == null && _ex == null)
		{
		    try
		    {
			wait(timeout);
			break;
		    }
		    catch(InterruptedException ex)
		    {
			continue;
		    }
		}

		if(_ex != null)
		{
		    throw _ex;
		}

		fd = _fd;
		_fd = null;
	    }

	    return fd;
	}

	private InetSocketAddress _addr;
	private java.net.Socket _fd;
	private java.io.IOException _ex;
    }

    public Transceiver
    connect(int timeout)
    {
	if(_traceLevels.network >= 2)
	{
	    String s = "trying to establish tcp connection to " + toString();
	    _logger.trace(_traceLevels.networkCat, s);
	}

	java.net.Socket fd = null;
	try
	{
	    //
	    // If a connect timeout is specified, do the connect in a separate thread.
	    //
	    if(timeout >= 0)
	    {
		ConnectThread ct = new ConnectThread(_addr);
		ct.start();
		fd = ct.getFd(timeout == 0 ? 1 : timeout);
		if(fd == null)
		{
		    throw new Ice.ConnectTimeoutException();
		}
	    }
	    else
	    {
		fd = new java.net.Socket(_addr.getAddress(), _addr.getPort());
	    }
            Network.setTcpBufSize(fd, _instance.initializationData().properties, _logger);
	}
        catch(java.net.ConnectException ex)
        {
	    if(fd != null)
	    {
		try
		{
		    fd.close();
		}
		catch(java.io.IOException e)
		{
		}
	    }
            Ice.ConnectFailedException se;
	    if(Network.connectionRefused(ex))
	    {
		se = new Ice.ConnectionRefusedException();
	    }
	    else
	    {
		se = new Ice.ConnectFailedException();
	    }
            se.initCause(ex);
            throw se;
        }
	catch(java.io.IOException ex)
	{
	    if(fd != null)
	    {
		try
		{
		    fd.close();
		}
		catch(java.io.IOException e)
		{
		}
	    }
	    Ice.SocketException e = new Ice.SocketException();
	    e.initCause(ex);
	    throw e;
	}
	catch(RuntimeException ex)
	{
	    if(fd != null)
	    {
		try
		{
		    fd.close();
		}
		catch(java.io.IOException e)
		{
		}
	    }
	    throw ex;
	}

	if(_traceLevels.network >= 1)
	{
	    String s = "tcp connection established\n" + IceInternal.Network.fdToString(fd);
	    _logger.trace(_traceLevels.networkCat, s);
	}

	return new Transceiver(_instance, fd);
    }

    public String
    toString()
    {
        return Network.addrToString(_addr);
    }

    //
    // Only for use by TcpEndpoint
    //
    Connector(Instance instance, String host, int port)
    {
        _instance = instance;
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;

        _addr = Network.getAddress(host, port);
    }

    private Instance _instance;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private InetSocketAddress _addr;
}
