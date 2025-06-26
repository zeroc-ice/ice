// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Provides access to an UDP endpoint information.
 *
 * @see Endpoint
 */
public final class UDPEndpointInfo extends IPEndpointInfo {
    /** The multicast interface. */
    public final String mcastInterface;

    /** The multicast time-to-live (or hops). */
    public final int mcastTtl;

    @Override
    public short type() {
        return UDPEndpointType.value;
    }

    @Override
    public boolean datagram() {
        return true;
    }

    // internal constructor
    UDPEndpointInfo(
            boolean compress,
            String host,
            int port,
            String sourceAddress,
            String mcastInterface,
            int mcastTtl) {
        super(compress, host, port, sourceAddress);
        this.mcastInterface = mcastInterface;
        this.mcastTtl = mcastTtl;
    }
}
