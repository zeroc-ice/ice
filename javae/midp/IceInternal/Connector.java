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
	ConnectThread(String addr)
	{
	    _url = addr;
	}

	public void
	run()
	{
	    try
	    {
		javax.microedition.io.Connection connection =
		    javax.microedition.io.Connector.open(_url,  javax.microedition.io.Connector.READ_WRITE,
							 true);
		synchronized(this)
		{
		    _connection = connection;
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

	javax.microedition.io.Connection
	getConnection(int timeout)
	    throws java.io.IOException
	{
	    javax.microedition.io.Connection connection = null;

	    synchronized(this)
	    {
		while(_connection == null && _ex == null)
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

		connection = _connection;
		_connection = null;
	    }

	    return connection;
	}

	private String _url;
	private java.io.IOException _ex;
	private javax.microedition.io.Connection _connection;
    }

    public Transceiver
    connect(int timeout)
    {
	if(_traceLevels.network >= 2)
	{
	    String s = "trying to establish tcp connection to " + toString();
	    _logger.trace(_traceLevels.networkCat, s);
	}

	javax.microedition.io.SocketConnection connection = null;
	try
	{
	    //
	    // If a connect timeout is specified, do the connect in a separate thread.
	    //
	    if(timeout >= 0)
	    {
		ConnectThread ct = new ConnectThread(_url);
		ct.start();
		connection = (javax.microedition.io.SocketConnection)ct.getConnection(timeout == 0 ? 1 : timeout);
		if(connection == null)
		{
		    throw new Ice.ConnectTimeoutException();
		}
	    }
	    else
	    {
		connection = (javax.microedition.io.SocketConnection)javax.microedition.io.Connector.open(_url, 
			javax.microedition.io.Connector.READ_WRITE, true);
	    }

	    connection.setSocketOption(javax.microedition.io.SocketConnection.DELAY, 0);
	}
        catch(javax.microedition.io.ConnectionNotFoundException ex)
        {
	    if(connection != null)
	    {
		try
		{
		    connection.close();
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
	    if(connection != null)
	    {
		try
		{
		    connection.close();
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
	    if(connection != null)
	    {
		try
		{
		    connection.close();
		}
		catch(java.io.IOException e)
		{
		}
	    }
	    throw ex;
	}

	if(_traceLevels.network >= 1)
	{
	    String s = "tcp connection established\n" + Network.toString(connection);
	    _logger.trace(_traceLevels.networkCat, s);
	}

	return new Transceiver(_instance, connection);
    }

    public String
    toString()
    {
	return _url;
    }

    //
    // Only for use by TcpEndpoint
    //
    Connector(Instance instance, String host, int port)
    {
        _instance = instance;
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;

	_url = "socket://" + host + ':' + port;
    }

    private Instance _instance;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private String _url;
}
