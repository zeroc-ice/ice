// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Provides access to a TCP endpoint information.
 *
 * @see Endpoint
 */
public final class TCPEndpointInfo extends IPEndpointInfo {
    private final short _type;
    private final boolean _secure;

    @Override
    public short type() {
        return _type;
    }

    @Override
    public boolean secure() {
        return _secure;
    }

    // internal constructor
    TCPEndpointInfo(
            int timeout,
            boolean compress,
            String host,
            int port,
            String sourceAddress,
            short type,
            boolean secure) {
        super(timeout, compress, host, port, sourceAddress);
        this._type = type;
        this._secure = secure;
    }
}
