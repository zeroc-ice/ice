// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Provides access to the address details of a IP endpoint.
 *
 * @see Endpoint
 */
public abstract class IPEndpointInfo extends EndpointInfo {
    /** The host or address configured with the endpoint. */
    public String host = "";

    /** The port number. */
    public int port;

    /** The source IP address. */
    public String sourceAddress = "";
}
