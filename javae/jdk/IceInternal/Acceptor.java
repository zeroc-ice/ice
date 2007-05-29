// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

class Acceptor
{
    public void
    close()
    {
        if(_traceLevels.network >= 1)
        {
            String s = "stopping to accept tcp connections at " + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        java.net.ServerSocket fd;
	synchronized(this)
        {
	    fd = _fd;
            _fd = null;
	}
        if(fd != null)
        {
            try
            {
                fd.close();
            }
            catch(java.io.IOException ex)
            {
                // Ignore.
            }
        }
    }

    public void
    listen()
    {
        // Nothing to do.

        if(_traceLevels.network >= 1)
        {
            String s = "accepting tcp connections at " + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }
    }

    public Transceiver
    accept(int timeout)
    {
	java.net.Socket fd = null;
	try
	{
	    if(timeout == -1)
	    {
		timeout = 0; // Infinite
	    }
	    else if(timeout == 0)
	    {
		timeout = 1;
	    }
	    _fd.setSoTimeout(timeout);
	    fd = _fd.accept();
            Network.setTcpBufSize(fd, _instance.initializationData().properties, _logger);
	}
	catch(java.io.InterruptedIOException ex)
	{
	    Ice.TimeoutException e = new Ice.TimeoutException();
	    e.initCause(ex);
	    throw e;
	}
	catch(java.io.IOException ex)
	{
	    Ice.SocketException e = new Ice.SocketException();
	    e.initCause(ex);
	    throw e;
	}

	if(_traceLevels.network >= 1)
	{
	    String s = "accepted tcp connection\n" + Network.fdToString(fd);
	    _logger.trace(_traceLevels.networkCat, s);
	}

	return new Transceiver(_instance, fd);
    }

    public void
    connectToSelf()
    {
	try
	{
	    java.net.Socket fd = new java.net.Socket(_addr.getAddress(), _addr.getPort());
	    fd.close();
	}
	catch(java.io.IOException ex)
	{
	    Ice.SocketException e = new Ice.SocketException();
	    e.initCause(ex);
	    throw e;
	}
    }

    public String
    toString()
    {
        return Network.addrToString(_addr);
    }

    int
    effectivePort()
    {
        return _addr.getPort();
    }

    Acceptor(Instance instance, String host, int port)
    {
        _instance = instance;
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;
        _backlog = 0;

        if(_backlog <= 0)
        {
            _backlog = 5;
        }

        try
        {
	    _addr = Network.getAddress(host, port);
	    if(_traceLevels.network >= 2)
	    {
		String s = "attempting to bind to tcp socket " + toString();
		_logger.trace(_traceLevels.networkCat, s);
	    }
	    _fd = new java.net.ServerSocket(port, _backlog, _addr.getAddress());
	    _addr = new InetSocketAddress(_addr.getAddress(), _fd.getLocalPort());
            if(!System.getProperty("os.name").startsWith("Windows"))
            {
                _fd.setReuseAddress(true);
            }
            Network.setTcpBufSize(_fd, _instance.initializationData().properties, _logger);
        }
        catch(java.io.IOException ex)
        {
	    if(_fd != null)
	    {
		try
		{
		    _fd.close();
		}
		catch(java.io.IOException e)
		{
		}
		_fd = null;
	    }
	    Ice.SocketException e = new Ice.SocketException();
	    e.initCause(ex);
	    throw e;
        }
        catch(RuntimeException ex)
        {
	    if(_fd != null)
	    {
		try
		{
		    _fd.close();
		}
		catch(java.io.IOException e)
		{
		}
		_fd = null;
	    }
            throw ex;
        }
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtil.Debug.FinalizerAssert(_fd == null);
    }

    private Instance _instance;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private java.net.ServerSocket _fd;
    private int _backlog;
    private InetSocketAddress _addr;
}
