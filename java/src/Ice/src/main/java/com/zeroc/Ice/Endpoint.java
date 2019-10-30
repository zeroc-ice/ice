//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * The user-level interface to an endpoint.
 **/
public interface Endpoint
{
    /**
     * Return a string representation of the endpoint.
     * @return The string representation of the endpoint.
     **/
    String _toString();

    /**
     * Returns the endpoint information.
     * @return The endpoint information class.
     **/
    EndpointInfo getInfo();
}
