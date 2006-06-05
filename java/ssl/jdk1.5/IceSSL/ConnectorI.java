// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class ConnectorI implements IceInternal.Connector
{
    public IceInternal.Transceiver
    connect(int timeout)
    {
	//
	// The plugin may not be fully initialized.
	//
	if(!_instance.initialized())
	{
	    Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
	    ex.reason = "IceSSL: plugin is not initialized";
	    throw ex;
	}

	if(_instance.networkTraceLevel() >= 2)
	{
	    String s = "trying to establish ssl connection to " + toString();
	    _logger.trace(_instance.networkTraceCategory(), s);
	}

	java.nio.channels.SocketChannel fd = IceInternal.Network.createTcpSocket();
	IceInternal.Network.setBlock(fd, false);
	IceInternal.Network.doConnect(fd, _addr, timeout);

	TransceiverI transceiver = null;
	try
	{
	    javax.net.ssl.SSLEngine engine = _instance.createSSLEngine(false);

	    transceiver = new TransceiverI(_instance, engine, fd, _host, false, "");
/*
	    transceiver.waitForHandshake(timeout);

	    //
	    // Check IceSSL.VerifyPeer.
	    //
	    int verifyPeer =
		_instance.communicator().getProperties().getPropertyAsIntWithDefault("IceSSL.VerifyPeer", 2);
	    if(verifyPeer > 0)
	    {
		try
		{
		    engine.getSession().getPeerCertificates();
		}
		catch(javax.net.ssl.SSLPeerUnverifiedException ex)
		{
		    Ice.SecurityException e = new Ice.SecurityException();
		    e.reason = "IceSSL: server did not supply a certificate";
		    e.initCause(ex);
		    throw e;
		}
	    }
*/

/*
	    if(!ctx.verifyPeer(fd, _host, false))
	    {
		Ice.SecurityException ex = new Ice.SecurityException();
		ex.reason = "IceSSL: outgoing connection rejected by certificate verifier";
		throw ex;
	    }
*/
	}
	catch(RuntimeException ex)
	{
	    try
	    {
		fd.close();
	    }
	    catch(java.io.IOException e)
	    {
		// Ignore.
	    }
	    throw ex;
	}

	if(_instance.networkTraceLevel() >= 1)
	{
	    String s = "ssl connection established\n" + IceInternal.Network.fdToString(fd);
	    _logger.trace(_instance.networkTraceCategory(), s);
	}

	return transceiver;
    }

    public String
    toString()
    {
	return IceInternal.Network.addrToString(_addr);
    }

    //
    // Only for use by EndpointI.
    //
    ConnectorI(Instance instance, String host, int port)
    {
	_instance = instance;
	_host = host;
	_logger = instance.communicator().getLogger();

	_addr = IceInternal.Network.getAddress(host, port);
    }

    private Instance _instance;
    private String _host;
    private Ice.Logger _logger;
    private java.net.InetSocketAddress _addr;
}
