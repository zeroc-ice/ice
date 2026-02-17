// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

import java.security.cert.Certificate;

/** Provides access to the connection details of an SSL connection */
public final class ConnectionInfo extends com.zeroc.Ice.ConnectionInfo {
    /** The negotiated cipher suite. */
    public final String cipher;

    /** The certificate chain. */
    public final Certificate[] certs;

    /** The certificate chain verification status. */
    public final boolean verified;

    ConnectionInfo(com.zeroc.Ice.ConnectionInfo underlying, String cipher, Certificate[] certs, boolean verified) {
        super(underlying);
        this.cipher = cipher;
        this.certs = certs != null ? certs.clone() : null;
        this.verified = verified;
    }
}
