// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

/**
 * This class provides information about a connection to applications
 * that require information about a peer, for example, to implement
 * a CertificateVerifier.
 *
 * @see CertificateVerifier
 **/
public class ConnectionInfo
{
    /**
     * The certificate chain. This may be null if the peer did not
     * supply a certificate. The peer's certificate (if any) is the
     * first one in the chain.
     **/
    public java.security.cert.Certificate[] certs;

    /**
     * The name of the negotiated cipher.
     **/
    public String cipher;

    /**
     * The local TCP/IP host & port.
     **/
    public java.net.InetSocketAddress localAddr;

    /**
     * The remote TCP/IP host & port.
     **/
    public java.net.InetSocketAddress remoteAddr;

    /**
     * <code>true</code> if the connection is incoming; <code>false</code> otherwise.
     **/
    public boolean incoming;

    /**
     * The name of the object adapter that hosts this endpoint, if any.
     **/
    public String adapterName;
}
