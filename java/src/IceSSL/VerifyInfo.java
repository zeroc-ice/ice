// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

//
// VerifyInfo contains information that may be of use to a
// CertificateVerifier implementation.
//
public class VerifyInfo
{
    // 
    // A value of true indicates an incoming (server) connection.
    //
    public boolean incoming;

    //
    // The peer's certificate chain, which can be null if the peer
    // is unverified.
    //
    public java.security.cert.Certificate[] certs;

    //
    // The SSL socket that is being authenticated.
    //
    public javax.net.ssl.SSLSocket socket;

    // 
    // The address of the server as specified by the proxy's
    // endpoint. For example, in the following proxy:
    // 
    // identity:ssl -h www.server.com -p 10000
    // 
    // the value of address is "www.server.com".
    //
    // The value is an empty string for incoming connections.
    //
    public String address;
}
