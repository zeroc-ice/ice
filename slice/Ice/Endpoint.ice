// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/Version.ice>
#include <Ice/BuiltinSequences.ice>
#include <Ice/EndpointF.ice>

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * Uniquely identifies TCP endpoints.
 *
 **/
const short TCPEndpointType = 1;

/**
 *
 * Uniquely identifies SSL endpoints.
 *
 **/
const short SSLEndpointType = 2;

/**
 *
 * Uniquely identifies UDP endpoints.
 *
 **/
const short UDPEndpointType = 3;

/**
 *
 * Uniquely identifies TCP-based WebSocket endpoints.
 *
 **/
const short WSEndpointType = 4;

/**
 *
 * Uniquely identifies SSL-based WebSocket endpoints.
 *
 **/
const short WSSEndpointType = 5;

/**
 *
 * Base class providing access to the endpoint details.
 *
 **/
local class EndpointInfo
{
    /**
     *
     * The timeout for the endpoint in milliseconds. 0 means
     * non-blocking, -1 means no timeout.
     *
     **/
    int timeout;

    /**
     *
     * Specifies whether or not compression should be used if
     * available when using this endpoint.
     *
     */
    bool compress;

    /**
     *
     * Returns the type of the endpoint.
     *
     **/
    ["cpp:const"] short type();

    /**
     *
     * Returns true if this endpoint is a datagram endpoint.
     *
     **/
    ["cpp:const"] bool datagram();

    /**
     *
     * Returns true if this endpoint is a secure endpoint.
     *
     **/
    ["cpp:const"] bool secure();
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

    /**
     *
     * Returns the endpoint information.
     *
     * @return The endpoint information class.
     *
     **/
    ["cpp:const"] EndpointInfo getInfo();
};

/**
 *
 * Provides access to the address details of a IP endpoint.
 *
 * @see Endpoint
 *
 **/
local class IPEndpointInfo extends EndpointInfo
{
    /**
     *
     * The host or address configured with the endpoint.
     *
     **/
    string host;

    /**
     *
     * The port number.
     *
     **/
    int port;

    /**
     *
     * The source IP address.
     *
     */
    string sourceAddress;
};

/**
 *
 * Provides access to a TCP endpoint information.
 *
 * @see Endpoint
 *
 **/
local class TCPEndpointInfo extends IPEndpointInfo
{
};

/**
 *
 * Provides access to an UDP endpoint information.
 *
 * @see Endpoint
 *
 **/
local class UDPEndpointInfo extends IPEndpointInfo
{
    /**
     *
     * The multicast interface.
     *
     **/
    string mcastInterface;

    /**
     *
     * The multicast time-to-live (or hops).
     *
     **/
     int mcastTtl;
};

/**
 *
 * Provides access to a WebSocket endpoint information.
 *
 **/
local class WSEndpointInfo extends TCPEndpointInfo
{
    /**
     *
     * The URI configured with the endpoint.
     *
     **/
    string resource;
};

/**
 *
 * Provides access to the details of an opaque endpoint.
 *
 * @see Endpoint
 *
 **/
local class OpaqueEndpointInfo extends EndpointInfo
{
    /**
     *
     * The encoding version of the opaque endpoint (to decode or
     * encode the rawBytes).
     *
     **/
    Ice::EncodingVersion rawEncoding;

    /**
     *
     * The raw encoding of the opaque endpoint.
     *
     **/
    Ice::ByteSeq rawBytes;
};

};

