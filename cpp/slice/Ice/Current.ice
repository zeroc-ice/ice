// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_CURRENT_ICE
#define ICE_CURRENT_ICE

#include <Ice/ObjectAdapterF.ice>
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
 * The [OperationMode] determines the skeleton
 * signature (for C++), as well as the retry
 * behavior of the Ice run time for an operation
 * invocation in case of a (potentially) recoverable
 * error.
 *
 **/
//
// Note: The order of definitions here *must* match the order of
// defininitions for ::Slice::Operation::Mode in include/Slice/Parser.h!
//
enum OperationMode
{
    /**
     * Ordinary operations have [Normal] mode. 
     * These operations modify object state;
     * invoking such an operation twice in a row
     * has different semantics than invoking it
     * once. The Ice run time guarantees that it
     * will not violate at-most-once semantics for
     * such operations.
     */
    Normal,

    /**
     * Operations that use the Slice nonmutating
     * keyword must not modify object state. For C++,
     * nonmutating operations generate [const]
     * member functions in the skeleton. In addition,
     * the Ice run time will attempt to transparently
     * recover from certain run-time errors by re-issuing
     * a failed request and propagate the failure to
     * the application only if the second attempt fails.
     */
    \Nonmutating,

    /**
     * Operations that use the Slice idempotent
     * keyword can modify object state, but invoking
     * an operation twice in a row must result in
     * the same object state as invoking it once.
     * For example, x = 1 is an idempotent statement,
     * whereas x += 1 is not. For idempotent
     * operations, the Ice run-time uses the same
     * retry behavior operations as for nonmutating
     * operations in case of a potentially recoverable
     * error.
     */
    \Idempotent
};

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
     * The object adapter.
     *
     **/
    ObjectAdapter adapter;

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
     **/
    Context ctx;
};

};

#endif
