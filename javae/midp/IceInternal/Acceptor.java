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
	if(_traceLevels.network >=1)
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
	    String s = "accepted tcp connection " + Network.toString(incoming);
	    _logger.trace(_traceLevels.networkCat, s);
	}

	return new Transceiver(_instance, incoming);
    }

    public void
    connectToSelf()
    {
	String ip = System.getProperty("microedition.hostname");
	if(ip == null || ip.length() == 0 || ip.equals("0.0.0.0"))
	{
	    try
	    {
		ip = _connection.getLocalAddress();
	    }
	    catch(java.io.IOException ex)
	    {
		ip = "127.0.0.1"; 
	    }
	}

	try
	{
	    javax.microedition.io.Connection localConn =
		javax.microedition.io.Connector.open("socket://" + ip + ':' + _connection.getLocalPort());
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

    int
    effectivePort()
    {
        return _addr.getPort();
    }

    //
    // The host argument is ignored in the MIDP profile.
    //
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

	String connectString = "socket://";
	if(port > 0)
	{
	    connectString = connectString + ":" + port;
	}

        try
        {
	    if(_traceLevels.network >= 2)
	    {
		String s = "attempting to bind to tcp socket on port " + port;
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
