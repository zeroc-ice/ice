// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Base class providing access to the endpoint details. */
public class EndpointInfo {
    /** The information of the underlying endpoint or null if there's no underlying endpoint. */
    public final EndpointInfo underlying;

    /** The timeout for the endpoint in milliseconds. 0 means non-blocking, -1 means no timeout. */
    public final int timeout;

    /**
     * Specifies whether or not compression should be used if available when using this endpoint.
     */
    public final boolean compress;

    /**
     * Returns the type of the endpoint.
     *
     * @return The endpoint type.
     */
    public short type() {
        return underlying != null ? underlying.type() : -1;
    }

    /**
     * Returns true if this endpoint is a datagram endpoint.
     *
     * @return True for a datagram endpoint.
     */
    public boolean datagram() {
        return underlying != null ? underlying.datagram() : false;
    }

    /**
     * @return True for a secure endpoint.
     */
    public boolean secure() {
        return underlying != null ? underlying.secure() : false;
    }

    protected EndpointInfo(EndpointInfo underlying) {
        assert underlying != null;
        this.underlying = underlying;
        this.timeout = underlying.timeout;
        this.compress = underlying.compress;
    }

    protected EndpointInfo(int timeout, boolean compress) {
        this.underlying = null;
        this.timeout = timeout;
        this.compress = compress;
    }
}
