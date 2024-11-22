// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Provides access to an UDP endpoint information.
 *
 * @see Endpoint
 */
public abstract class UDPEndpointInfo extends IPEndpointInfo {
    /** The multicast interface. */
    public String mcastInterface = "";

    /** The multicast time-to-live (or hops). */
    public int mcastTtl;
}
