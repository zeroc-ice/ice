// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CURRENT_ICE
#define ICE_CURRENT_ICE

#include <Ice/Identity.ice>
#include <Ice/Facet.ice>

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
 * implicitly. [Current] is mostly used for &Ice; services, such as
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
     * The &Ice; object identity.
     *
     **/
    Identity id;

    /**
     *
     * The facet.
     *
     ***/
    FacetPath facet;

    /**
     *
     * The operation name.
     *
     **/
    string operation;

    /**
     *
     * Set to true if the operation is idempotent.
     *
     **/
    bool isIdempotent;

    /**
     *
     * The request context, as received from the client.
     *
     * @see Context;
     *
     **/
    Context ctx;
};

};

#endif
