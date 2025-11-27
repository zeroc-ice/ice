// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Base class for the endpoint info classes. */
public class EndpointInfo {
    /** The information of the underlying endpoint or {@code null} if there's no underlying endpoint. */
    public final EndpointInfo underlying;

    /** Specifies whether or not compression should be used if available when using this endpoint. */
    public final boolean compress;

    /**
     * Returns the type of the endpoint.
     *
     * @return the endpoint type
     */
    public short type() {
        return underlying != null ? underlying.type() : -1;
    }

    /**
     * Returns whether this endpoint is a datagram endpoint.
     *
     * @return {@code true} for a UDP endpoint, {@code false} otherwise.
     */
    public boolean datagram() {
        return underlying != null ? underlying.datagram() : false;
    }

    /**
     * Returns whether this endpoint uses SSL.
     *
     * @returns {@code true} if this endpoint's transport uses SSL, {@code false} otherwise.
     */
    public boolean secure() {
        return underlying != null ? underlying.secure() : false;
    }

    /**
     * Constructs an EndpointInfo with an underlying endpoint.
     *
     * @param underlying the underlying EndpointInfo. Must not be null.
     */
    protected EndpointInfo(EndpointInfo underlying) {
        assert underlying != null;
        this.underlying = underlying;
        this.compress = underlying.compress;
    }

    /**
     * Constructs an EndpointInfo with the specified compression flag.
     *
     * @param compress {@code true} if compression should be used when available
     */
    protected EndpointInfo(boolean compress) {
        this.underlying = null;
        this.compress = compress;
    }
}
