// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Base class providing access to the endpoint details. */
public class EndpointInfo {
    /** The information of the underlying endpoint or null if there's no underlying endpoint. */
    public final EndpointInfo underlying;

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

    /**
     * Constructs an EndpointInfo with an underlying endpoint.
     *
     * @param underlying The underlying EndpointInfo. Must not be null.
     */
    protected EndpointInfo(EndpointInfo underlying) {
        assert underlying != null;
        this.underlying = underlying;
        this.compress = underlying.compress;
    }

    /**
     * Constructs an EndpointInfo with the specified compression flag.
     *
     * @param compress True if compression should be used if available.
     */
    protected EndpointInfo(boolean compress) {
        this.underlying = null;
        this.compress = compress;
    }
}
