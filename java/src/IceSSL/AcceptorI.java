// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

class AcceptorI implements IceInternal.Acceptor
{
    public java.nio.channels.ServerSocketChannel
    fd()
    {
	return null;
    }

    public void
    close()
    {
	if(_instance.networkTraceLevel() >= 1)
	{
	    String s = "stopping to accept ssl connections at " + toString();
	    _logger.trace(_instance.networkTraceCategory(), s);
	}

	javax.net.ssl.SSLServerSocket fd;
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

	if(_instance.networkTraceLevel() >= 1)
	{
	    String s = "accepting ssl connections at " + toString();
	    _logger.trace(_instance.networkTraceCategory(), s);
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

	if(_instance.networkTraceLevel() >= 1)
	{
	    String s = "accepted ssl connection\n" + IceInternal.Network.fdToString(fd);
	    _logger.trace(_instance.networkTraceCategory(), s);
	}

	return new TransceiverI(_instance, fd);
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

    AcceptorI(Instance instance, String host, int port)
    {
	_instance = instance;
	_ctx = instance.serverContext();
	_logger = instance.communicator().getLogger();
	_backlog = 0;

	if(_backlog <= 0)
	{
	    _backlog = 5;
	}

	try
	{
	    javax.net.ssl.SSLServerSocketFactory factory = _ctx.sslContext().getServerSocketFactory();
	    _addr = new java.net.InetSocketAddress(host, port);
	    if(_instance.networkTraceLevel() >= 2)
	    {
		String s = "attempting to bind to ssl socket " + toString();
		_logger.trace(_instance.networkTraceCategory(), s);
	    }
	    java.net.InetAddress iface = java.net.InetAddress.getByName(host);
	    _fd = (javax.net.ssl.SSLServerSocket)factory.createServerSocket(port, _backlog, iface);
	    _addr = (java.net.InetSocketAddress)_fd.getLocalSocketAddress();

	    int verifyPeer = _instance.communicator().getProperties().getPropertyAsIntWithDefault(
		"IceSSL.Server.VerifyPeer", -1);
	    if(verifyPeer == -1)
	    {
		//
		// Check deprecated ClientAuth property.
		//
		verifyPeer = _instance.communicator().getProperties().getPropertyAsIntWithDefault(
		    "IceSSL.Server.ClientAuth", 2);
	    }
	    if(verifyPeer == 0)
	    {
		_fd.setWantClientAuth(false);
		_fd.setNeedClientAuth(false);
	    }
	    else if(verifyPeer == 1)
	    {
		_fd.setWantClientAuth(true);
	    }
	    else
	    {
		_fd.setNeedClientAuth(true);
	    }

	    String[] cipherSuites = _ctx.filterCiphers(_fd.getSupportedCipherSuites(), _fd.getEnabledCipherSuites());
	    if(_instance.securityTraceLevel() > 0)
	    {
		StringBuffer s = new StringBuffer();
		s.append("enabling ciphersuites for ssl server socket " + toString() + ":");
		for(int i = 0; i < cipherSuites.length; ++i)
		{
		    s.append("\n  " + cipherSuites[i]);
		}
		_logger.trace(_instance.securityTraceCategory(), s.toString());
	    }
	    _fd.setEnabledCipherSuites(cipherSuites);
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

    private Instance _instance;
    private Context _ctx;
    private Ice.Logger _logger;
    private javax.net.ssl.SSLServerSocket _fd;
    private int _backlog;
    private java.net.InetSocketAddress _addr;
}
