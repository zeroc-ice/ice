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

    /**
     * Constructs an IPEndpointInfo with the specified parameters.
     *
     * @param timeout the timeout in milliseconds
     * @param compress whether to compress the endpoint
     * @param host the host or address configured with the endpoint
     * @param port the port number
     * @param sourceAddress the source IP address
     */
    protected IPEndpointInfo(
            int timeout, boolean compress, String host, int port, String sourceAddress) {
        super(timeout, compress);
        this.host = host;
        this.port = port;
        this.sourceAddress = sourceAddress;
    }
}
