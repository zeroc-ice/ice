// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * An endpoint specifies the address of the server-end of an Ice connection.
 * An object adapter listens on one or more endpoints and a client establishes a connection to an endpoint.
 */
public interface Endpoint {
    /**
     * Returns a string representation of this endpoint.
     *
     * @return the string representation of this endpoint
     */
    String _toString();

    /**
     * Returns this endpoint's information.
     *
     * @return this endpoint's information class
     */
    EndpointInfo getInfo();
}
