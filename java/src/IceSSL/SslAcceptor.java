// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

class SslAcceptor implements IceInternal.Acceptor
{
    public java.nio.channels.ServerSocketChannel
    fd()
    {
	return null;
    }

    public void
    close()
    {
	if(_traceLevels.network >= 1)
	{
	    String s = "stopping to accept ssl connections at " + toString();
	    _logger.trace(_traceLevels.networkCat, s);
	}

	javax.net.ssl.SSLServerSocket fd = _fd;
	if(fd != null)
	{
	    _fd = null;
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
	    String s = "accepting ssl connections at " + toString();
	    _logger.trace(_traceLevels.networkCat, s);
	}
    }

    public IceInternal.Transceiver
    accept(int timeout)
    {
	javax.net.ssl.SSLSocket fd = null;
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
	    fd = (javax.net.ssl.SSLSocket)_fd.accept();
	    fd.setUseClientMode(false);
	}
	catch(java.net.SocketTimeoutException ex)
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
	    String s = "accepted ssl connection\n" + IceInternal.Network.fdToString(fd);
	    _logger.trace(_traceLevels.networkCat, s);
	}

	return new SslTransceiver(_instance, _ctx, fd);
    }

    public void
    connectToSelf()
    {
	java.nio.channels.SocketChannel fd = IceInternal.Network.createTcpSocket();
	IceInternal.Network.setBlock(fd, false);
	IceInternal.Network.doConnect(fd, _addr, -1);
	IceInternal.Network.closeSocket(fd);
    }

    public String
    toString()
    {
	return IceInternal.Network.addrToString(_addr);
    }

    final boolean
    equivalent(String host, int port)
    {
	java.net.InetSocketAddress addr = IceInternal.Network.getAddress(host, port);
	return addr.equals(_addr);
    }

    int
    effectivePort()
    {
	return _addr.getPort();
    }

    SslAcceptor(IceInternal.Instance instance, javax.net.ssl.SSLContext ctx, String host, int port)
    {
	_instance = instance;
	_ctx = ctx;
	_traceLevels = instance.traceLevels();
	_logger = instance.logger();
	_backlog = 0;

	if(_backlog <= 0)
	{
	    _backlog = 5;
	}

	try
	{
	    javax.net.ssl.SSLServerSocketFactory factory = _ctx.getServerSocketFactory();
	    _addr = new java.net.InetSocketAddress(host, port);
	    if(_traceLevels.network >= 2)
	    {
		String s = "attempting to bind to ssl socket " + toString();
		_logger.trace(_traceLevels.networkCat, s);
	    }
	    java.net.InetAddress iface = java.net.InetAddress.getByName(host);
	    _fd = (javax.net.ssl.SSLServerSocket)factory.createServerSocket(port, _backlog, iface);
	    _addr = (java.net.InetSocketAddress)_fd.getLocalSocketAddress();
String[] suite = new String[]{ "SSL_DH_anon_WITH_DES_CBC_SHA" };
_fd.setEnabledCipherSuites(suite);
	}
	catch(java.io.IOException ex)
	{
	    try
	    {
		if(_fd != null)
		{
		    _fd.close();
		}
	    }
	    catch(java.io.IOException e)
	    {
	    }
	    _fd = null;
	    Ice.SocketException se = new Ice.SocketException();
	    se.initCause(ex);
	    throw se;
	}
    }

    protected void
    finalize()
	throws Throwable
    {
	assert(_fd == null);

	super.finalize();
    }

    private IceInternal.Instance _instance;
    private javax.net.ssl.SSLContext _ctx;
    private IceInternal.TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private javax.net.ssl.SSLServerSocket _fd;
    private int _backlog;
    private java.net.InetSocketAddress _addr;
}
