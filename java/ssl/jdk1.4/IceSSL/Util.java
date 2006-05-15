// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

public final class Util
{
    public static ConnectionInfo
    getConnectionInfo(Ice.Connection connection)
    {
	Ice.ConnectionI con = (Ice.ConnectionI)connection;
	assert(con != null);

	//
	// Lock the connection directly. This is done because the only
	// thing that prevents the transceiver from being closed during
	// the duration of the invocation is the connection.
	//
	synchronized(con)
	{
	    IceInternal.Transceiver transceiver = con.getTransceiver();
	    if(transceiver == null)
	    {
		ConnectionInvalidException ex = new ConnectionInvalidException();
		ex.reason = "connection closed";
		throw ex;
	    }

	    try
	    {
		TransceiverI sslTransceiver = (TransceiverI)transceiver;
		return sslTransceiver.getConnectionInfo();
	    }
	    catch(ClassCastException ex)
	    {
		ConnectionInvalidException e = new ConnectionInvalidException();
		e.reason = "not ssl connection";
		throw e;
	    }
	}
    }

    static ConnectionInfo
    populateConnectionInfo(javax.net.ssl.SSLSocket fd)
    {
	ConnectionInfo info = new ConnectionInfo();
	javax.net.ssl.SSLSession session = fd.getSession();
	try
	{
	    info.certs = session.getPeerCertificates();
	}
	catch(javax.net.ssl.SSLPeerUnverifiedException ex)
	{
	    // No peer certificates.
	}
	info.cipher = session.getCipherSuite();
	info.localAddr = (java.net.InetSocketAddress)fd.getLocalSocketAddress();
	info.remoteAddr = (java.net.InetSocketAddress)fd.getRemoteSocketAddress();
	return info;
    }
}
