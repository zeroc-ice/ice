// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Provides access to the address details of a IP endpoint.
 *
 * @see Endpoint
 */
public class IPEndpointInfo extends EndpointInfo {
    /** The host or address configured with the endpoint. */
    public final String host;

    /** The port number. */
    public final int port;

    /** The source IP address. */
    public final String sourceAddress;

    protected IPEndpointInfo(
            int timeout, boolean compress, String host, int port, String sourceAddress) {
        super(timeout, compress);
        this.host = host;
        this.port = port;
        this.sourceAddress = sourceAddress;
    }
}
