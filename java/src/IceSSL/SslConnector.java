// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class SslConnector implements IceInternal.Connector
{
    private static class ConnectThread extends Thread
    {
	ConnectThread(javax.net.ssl.SSLContext ctx, java.net.InetSocketAddress addr)
	{
	    _ctx = ctx;
	    _addr = addr;
	}

	protected void
	finalize()
	    throws Throwable
	{
	    if(_fd != null)
	    {
		try
		{
		    _fd.close();
		}
		catch(java.io.IOException ex)
		{
		}
	    }
	}

	public void
	run()
	{
	    try
	    {
		javax.net.SocketFactory factory = _ctx.getSocketFactory();
		javax.net.ssl.SSLSocket fd =
		    (javax.net.ssl.SSLSocket)factory.createSocket(_addr.getAddress(), _addr.getPort());
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

	javax.net.ssl.SSLSocket
	getFd(int timeout)
	    throws java.io.IOException
	{
	    javax.net.ssl.SSLSocket fd = null;

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

	private javax.net.ssl.SSLContext _ctx;
	private java.net.InetSocketAddress _addr;
	private javax.net.ssl.SSLSocket _fd;
	private java.io.IOException _ex;
    }

    private static class HandshakeThread extends Thread
    {
	HandshakeThread(javax.net.ssl.SSLSocket fd)
	{
	    _fd = fd;
	    _ok = false;
	}

	public void
	run()
	{
	    try
	    {
		_fd.startHandshake();
		synchronized(this)
		{
		    _ok = true;
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

	boolean
	waitForHandshake(int timeout)
	    throws java.io.IOException
	{
	    boolean result = false;

	    synchronized(this)
	    {
		while(!_ok && _ex == null)
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

		result = _ok;
	    }

	    return result;
	}

	private javax.net.ssl.SSLSocket _fd;
	private boolean _ok;
	private java.io.IOException _ex;
    }

    public IceInternal.Transceiver
    connect(int timeout)
    {
	if(_traceLevels.network >= 2)
	{
	    String s = "trying to establish ssl connection to " + toString();
	    _logger.trace(_traceLevels.networkCat, s);
	}

	javax.net.ssl.SSLSocket fd = null;
	try
	{
	    //
	    // If a connect timeout is specified, do the connect in a separate thread.
	    //
	    if(timeout >= 0)
	    {
		ConnectThread ct = new ConnectThread(_ctx, _addr);
		ct.start();
		fd = ct.getFd(timeout == 0 ? 1 : timeout);
		if(fd == null)
		{
		    throw new Ice.ConnectTimeoutException();
		}
	    }
	    else
	    {
		javax.net.SocketFactory factory = _ctx.getSocketFactory();
		fd = (javax.net.ssl.SSLSocket)factory.createSocket(_addr.getAddress(), _addr.getPort());
	    }

	    fd.setUseClientMode(true);

	    // TODO: Temporary
	    String[] suite = new String[]{ "SSL_DH_anon_WITH_DES_CBC_SHA" };
	    fd.setEnabledCipherSuites(suite);

	    //
	    // If a connect timeout is specified, do the SSL handshake in a separate thread.
	    //
	    if(timeout >= 0)
	    {
		HandshakeThread ht = new HandshakeThread(fd);
		ht.start();
		if(!ht.waitForHandshake(timeout == 0 ? 1 : timeout))
		{
		    throw new Ice.ConnectTimeoutException();
		}
	    }
	    else
	    {
		fd.startHandshake();
	    }
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
	    if(IceInternal.Network.connectionRefused(ex))
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
	    String s = "ssl connection established\n" + IceInternal.Network.fdToString(fd);
	    _logger.trace(_traceLevels.networkCat, s);
	}

	/*
String[] suites = fd.getSupportedCipherSuites();
System.out.println("Supported cipher suites:");
for(int i = 0; i < suites.length; ++i)
{
    System.out.println("  " + suites[i]);
}
suites = fd.getEnabledCipherSuites();
System.out.println("Enabled cipher suites:");
for(int i = 0; i < suites.length; ++i)
{
    System.out.println("  " + suites[i]);
}
String[] protocols = fd.getSupportedProtocols();
System.out.println("Supported protocols:");
for(int i = 0; i < protocols.length; ++i)
{
    System.out.println("  " + protocols[i]);
}
	*/

	return new SslTransceiver(_instance, _ctx, fd);
    }

    public String
    toString()
    {
	return IceInternal.Network.addrToString(_addr);
    }

    //
    // Only for use by SslEndpoint
    //
    SslConnector(IceInternal.Instance instance, javax.net.ssl.SSLContext ctx, String host, int port)
    {
	_instance = instance;
	_ctx = ctx;
	_traceLevels = instance.traceLevels();
	_logger = instance.logger();

	_addr = IceInternal.Network.getAddress(host, port);
    }

    private IceInternal.Instance _instance;
    javax.net.ssl.SSLContext _ctx;
    private IceInternal.TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private java.net.InetSocketAddress _addr;
}
