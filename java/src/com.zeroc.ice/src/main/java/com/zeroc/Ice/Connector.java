// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * @hidden Public because it's used by IceBT, SSL, and the 'Ice/background' test.
 */
public interface Connector {
    Transceiver connect();

    short type();

    @Override
    String toString();

    //
    // Compare connectors for sorting process.
    //
    @Override
    boolean equals(java.lang.Object obj);
}
