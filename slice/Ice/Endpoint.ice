// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ENDPOINT_ICE
#define ICE_ENDPOINT_ICE

#include <Ice/BuiltinSequences.ice>
#include <Ice/EndpointF.ice>

[["cpp:header-ext:h"]]

module Ice
{

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

    /**
     *
     * Return the timeout for the endpoint in milliseconds.
     * 0 means non-blocking, -1 means no timeout.
     *
     * @return The timeout.
     *
     **/
    ["cpp:const"] int timeout();

    /**
     *
     * Check whether endpoint supports bzip2 compress,
     * or false otherwise.
     *
     * @return Whether compression is enabled.
     *
     */
    ["cpp:const"] bool compress();
};

/**
 *
 * A TCP endpoint.
 *
 * @see Endpoint
 *
 **/
local interface TcpEndpoint extends Endpoint
{
    /**
     * 
     * Get the host or address configured with
     * the endpoint.
     * 
     * @return The host or address.
     *
     **/
    ["cpp:const"] string host();

    /**
     * 
     * Get the TCP port number.
     *
     * @return The port number.
     * 
     **/
    ["cpp:const"] int port();
};

/**
 * 
 * A UDP endpoint.
 *
 * @see Endpoint
 *
 **/
local interface UdpEndpoint extends Endpoint
{
    /**
     * 
     * Get the host or address configured with
     * the endpoint.
     * 
     * @return The host or address.
     *
     **/
    ["cpp:const"] string host();

    /**
     * 
     * Get the TCP port number.
     *
     * @return The port number.
     * 
     **/
    ["cpp:const"] int port();
    
    /**
     * 
     * Get the multicast interface.
     *
     * @return The multicast interface.
     *
     **/
    ["cpp:const"] string mcastInterface();

    /**
     *
     * Get the multicast time-to-live (or hops).
     *
     * @return The time-to-live.
     * 
     **/
    ["cpp:const"] int mcastTtl();
};

/**
 * 
 * An opaque endpoint.
 *
 * @see Endpoint
 *
 **/
local interface OpaqueEndpoint extends Endpoint
{
    /**
     *
     * Return the raw encoding of the opaque endpoint.
     *
     * @return The raw encoding.
     *
     **/
    ["cpp:const"] Ice::ByteSeq rawBytes();
};

};

#endif
