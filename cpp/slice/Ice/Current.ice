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
 * The [OperationMode] indicates whether an operation
 * is an ordinary (mutating) operation, a nonmutating
 * (const) operation, or an idempotent (safely
 * reinvocable) operation.
 *
 **/
//
// Note: The order of definitions here *must* match the order of
// defininitions for ::Slice::Operation::Mode in include/Slice/Parser.h!
//
// TODO: ML: Coding conventions (placement of braces), and document the
// individual elements of the enum.
//
enum OperationMode { Normal, \Nonmutating, \Idempotent };

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
     * The mode of the operation.
     *
     **/
    OperationMode mode;

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
