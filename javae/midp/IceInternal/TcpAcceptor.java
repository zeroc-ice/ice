// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

class TcpAcceptor implements Acceptor
{
    public void
    close()
    {
        if(_traceLevels.network >= 1)
        {
            String s = "stopping to accept tcp connections at " + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

	javax.microedition.io.ServerSocketConnection connection;
	synchronized(this)
        {
	    connection = _connection;
            _connection = null;
	}
        if(connection != null)
        {
            try
            {
                connection.close();
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
	javax.microedition.io.SocketConnection incoming = null;
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
	    incoming = (javax.microedition.io.SocketConnection)_connection.acceptAndOpen();
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
	    String s = "accepted tcp connection\n" + Network.toString(incoming);
	    _logger.trace(_traceLevels.networkCat, s);
	}

	return new TcpTransceiver(_instance, incoming);
    }

    public void
    connectToSelf()
    {
	try
	{
	    javax.microedition.io.Connection localConn =
		javax.microedition.io.Connector.open("socket://" + _addr.getAddress() + ':' + _addr.getPort());
	    localConn.close();
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
	return _addr.getAddress() + ":" + _addr.getPort();
    }

    final boolean
    equivalent(String host, int port)
    {
        InetSocketAddress addr = new InetSocketAddress(host, port);
        return addr.equals(_addr);
    }

    int
    effectivePort()
    {
        return _addr.getPort();
    }

    //
    // The host argument is ignored in the MIDP profile.
    //
    TcpAcceptor(Instance instance, String host, int port)
    {
        _instance = instance;
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
        _backlog = 0;

        if(_backlog <= 0)
        {
            _backlog = 5;
        }

	String connectString = "socket://";
	if(port > 0)
	{
	    connectString = connectString + ":" + port;
	}

        try
        {
	    if(_traceLevels.network >= 2)
	    {
		String s = "attempting to bind to tcp socket " + toString();
		_logger.trace(_traceLevels.networkCat, s);
	    }
	    _connection =
		(javax.microedition.io.ServerSocketConnection)javax.microedition.io.Connector.open(connectString);

	    _addr = new InetSocketAddress(_connection.getLocalAddress(), _connection.getLocalPort());
        }
        catch(java.io.IOException ex)
        {
	    if(_connection != null)
	    {
		try
		{
		    _connection.close();
		}
		catch(java.io.IOException e)
		{
		}
		_connection = null;
	    }
	    Ice.SocketException e = new Ice.SocketException();
	    e.initCause(ex);
	    throw e;
        }
        catch(RuntimeException ex)
        {
	    if(_connection != null)
	    {
		try
		{
		    _connection.close();
		}
		catch(java.io.IOException e)
		{
		}
		_connection = null;
	    }
            throw ex;
        }
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtil.Debug.FinalizerAssert(_connection == null);
    }

    private Instance _instance;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private javax.microedition.io.ServerSocketConnection _connection;
    private int _backlog;
    private InetSocketAddress _addr;
}
