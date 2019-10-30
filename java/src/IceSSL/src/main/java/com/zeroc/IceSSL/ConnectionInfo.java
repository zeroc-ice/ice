//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceSSL;

/**
 * Provides access to the connection details of an SSL connection
 **/
public class ConnectionInfo extends com.zeroc.Ice.ConnectionInfo
{
    public ConnectionInfo()
    {
        super();
        this.cipher = "";
    }

    public ConnectionInfo(com.zeroc.Ice.ConnectionInfo underlying, boolean incoming, String adapterName, String connectionId, String cipher, java.security.cert.Certificate[] certs, boolean verified)
    {
        super(underlying, incoming, adapterName, connectionId);
        this.cipher = cipher;
        this.certs = certs;
        this.verified = verified;
    }

    /**
     * The negotiated cipher suite.
     **/
    public String cipher;

    /**
     * The certificate chain.
     **/
    public java.security.cert.Certificate[] certs;

    /**
     * The certificate chain verification status.
     **/
    public boolean verified;

    public ConnectionInfo clone()
    {
        return (ConnectionInfo)super.clone();
    }

    /** @hidden */
    public static final long serialVersionUID = 8227416448029738634L;
}
