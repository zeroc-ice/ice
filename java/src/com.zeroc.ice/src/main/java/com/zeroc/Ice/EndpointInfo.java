// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Base class providing access to the endpoint details. */
public abstract class EndpointInfo implements Cloneable {
    /** The information of the underlying endpoint or null if there's no underlying endpoint. */
    public EndpointInfo underlying;

    /** The timeout for the endpoint in milliseconds. 0 means non-blocking, -1 means no timeout. */
    public int timeout;

    /**
     * Specifies whether or not compression should be used if available when using this endpoint.
     */
    public boolean compress;

    /**
     * Returns the type of the endpoint.
     *
     * @return The endpoint type.
     */
    public abstract short type();

    /**
     * Returns true if this endpoint is a datagram endpoint.
     *
     * @return True for a datagram endpoint.
     */
    public abstract boolean datagram();

    /**
     * @return True for a secure endpoint.
     */
    public abstract boolean secure();

    @Override
    public EndpointInfo clone() {
        try {
            return (EndpointInfo) super.clone();
        } catch (CloneNotSupportedException ex) {
            assert false; // impossible
            return null;
        }
    }
}
