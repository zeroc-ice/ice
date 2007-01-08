// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ENDPOINT_ICE
#define ICE_ENDPOINT_ICE

module Ice
{

/**
 *
 * Determines the order in which the Ice run time uses the endpoints
 * in a proxy when establishing a connection.
 *
 **/
enum EndpointSelectionType
{
    /**
     * [Random] causes the endpoints to be arranged in a random order.
     */
    Random,
    /**
     * [Ordered] forces the Ice run time to use the endpoints in the
     * order they appeared in the proxy.
     */
    Ordered
};

/**
 *
 * The user-level interface to an endpoint.
 *
 **/
local interface Endpoint
{
    /**
     *
     * Return a string representation of the endpoint.
     *
     * @return The string representation of the endpoint.
     *
     **/
    ["cpp:const"] string toString();
};

/**
 * A sequence of [Endpoint] interfaces.
 **/
local sequence<Endpoint> EndpointSeq;

};

#endif
