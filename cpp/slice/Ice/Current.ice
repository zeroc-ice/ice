// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CURRENT_ICE
#define ICE_CURRENT_ICE

#include <Ice/ConnectionF.ice>
#include <Ice/Identity.ice>

module Ice
{

/**
 *
 * A request context. [Context] is used to transmit metadata about a
 * request from the server to the client, such as Quality-of-Service
 * (QoS) parameters. Each method on the client as a [Context] as last
 * parameter implicitly.
 *
 **/
local dictionary<string, string> Context;
    
/**
 *
 * Information about the current method invocation for servers. Each
 * method on the server has a [Current] as last parameter
 * implicitly. [Current] is mostly used for Ice services, such as
 * &IceStorm;. "Regular" user applications normally ignore this last
 * parameter.
 *
 * @see IceStorm
 *
 **/
local struct Current
{
    /**
     *
     * The Ice Object identity.
     *
     **/
    Identity identity;

    /**
     *
     * The facet.
     *
     ***/
    string facet;

    /**
     *
     * The operation name.
     *
     **/
    string operation;

    /**
     *
     * Set to true if the operation is nonmutating.
     *
     **/
    bool \nonmutating;

    /**
     *
     * The request context, as received from the client.
     *
     * @see Context;
     *
     **/
    Context context;

    /**
     *
     * The proxy via which this request was sent. If not available
     * (i.e., if set to null) the server can ask the client to repeat
     * the request with including a proxy by throwing an
     * [Ice::ProxyRequested] exception.
     *
     **/
    Object* proxy;

    /**
     *
     * The connection over which the request was received. In case no
     * connection was involved for propagating the request, like if
     * the request was sent directly by a collocated client,
     * [connection] is set to null.
     *
     **/
    Connection connection;
};

};

#endif
