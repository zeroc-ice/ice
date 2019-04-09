//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:dll-export:ICE_API"]]
[["objc:header-dir:objc"]]

[["python:pkgdir:Ice"]]

#include <Ice/ObjectAdapterF.ice>
#include <Ice/ConnectionF.ice>
#include <Ice/Identity.ice>
#include <Ice/Version.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * A request context. <code>Context</code> is used to transmit metadata about a
 * request from the server to the client, such as Quality-of-Service
 * (QoS) parameters. Each operation on the client has a <code>Context</code> as
 * its implicit final parameter.
 *
 **/
dictionary<string, string> Context;

/**
 *
 * Determines the retry behavior an invocation in case of a (potentially) recoverable error.
 *
 **/
//
// Note: The order of definitions here *must* match the order of
// definitions for ::Slice::Operation::Mode in include/Slice/Parser.h!
//
enum OperationMode
{
    /**
     * Ordinary operations have <code>Normal</code> mode.  These operations
     * modify object state; invoking such an operation twice in a row
     * has different semantics than invoking it once. The Ice run time
     * guarantees that it will not violate at-most-once semantics for
     * <code>Normal</code> operations.
     */
    Normal,

    /**
     * Operations that use the Slice <code>nonmutating</code> keyword must not
     * modify object state. For C++, nonmutating operations generate
     * <code>const</code> member functions in the skeleton. In addition, the Ice
     * run time will attempt to transparently recover from certain
     * run-time errors by re-issuing a failed request and propagate
     * the failure to the application only if the second attempt
     * fails.
     *
     * <p class="Deprecated"><code>Nonmutating</code> is deprecated; Use the
     * <code>idempotent</code> keyword instead. For C++, to retain the mapping
     * of <code>nonmutating</code> operations to C++ <code>const</code>
     * member functions, use the <code>\["cpp:const"]</code> metadata
     * directive.
     */
    \Nonmutating,

    /**
     * Operations that use the Slice <code>idempotent</code> keyword can modify
     * object state, but invoking an operation twice in a row must
     * result in the same object state as invoking it once.  For
     * example, <code>x = 1</code> is an idempotent statement,
     * whereas <code>x += 1</code> is not. For idempotent
     * operations, the Ice run-time uses the same retry behavior
     * as for nonmutating operations in case of a potentially
     * recoverable error.
     */
    \Idempotent
}

/**
 *
 * Information about the current method invocation for servers. Each
 * operation on the server has a <code>Current</code> as its implicit final
 * parameter. <code>Current</code> is mostly used for Ice services. Most
 * applications ignore this parameter.
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
     * Information about the connection over which the current method
     * invocation was received. If the invocation is direct due to
     * collocation optimization, this value is set to null.
     *
     **/
    Connection con;

    /**
     *
     * The Ice object identity.
     *
     **/
    Identity id;

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

    /**
     *
     * The request id unless oneway (0).
     *
     **/
    int requestId;

    /**
     *
     * The encoding version used to encode the input and output parameters.
     *
     **/
    EncodingVersion encoding;
}

}
