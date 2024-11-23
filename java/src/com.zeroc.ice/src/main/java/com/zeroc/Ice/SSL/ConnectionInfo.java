// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

/** Provides access to the connection details of an SSL connection */
public final class ConnectionInfo extends com.zeroc.Ice.ConnectionInfo {
    /** The negotiated cipher suite. */
    public String cipher = "";

    /** The certificate chain. */
    public java.security.cert.Certificate[] certs;

    /** The certificate chain verification status. */
    public boolean verified;
}
