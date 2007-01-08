// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

//
// ConnectionInfo contains information that may be of use to a
// CertificateVerifier or an application that wants information
// about its peer.
//
public class ConnectionInfo
{
    //
    // The certificate chain. This may be null if the peer did not
    // supply a certificate. The peer's certificate (if any) is the
    // first one in the chain.
    //
    public java.security.cert.Certificate[] certs;

    //
    // The name of the negotiated cipher.
    //
    public String cipher;

    //
    // The local TCP/IP host & port.
    //
    public java.net.InetSocketAddress localAddr;

    //
    // The remote TCP/IP host & port.
    //
    public java.net.InetSocketAddress remoteAddr;

    //
    // If the connection is incoming this bool is true, false
    // otherwise.
    //
    boolean incoming;

    //
    // The name of the object adapter that hosts this endpoint, if
    // any.
    //
    String adapterName;
}
