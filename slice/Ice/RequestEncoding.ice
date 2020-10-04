//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[cpp:dll-export:ICE_API]]
[[cpp:doxygen:include:Ice/Ice.h]]
[[cpp:header-ext:h]]

[[suppress-warning:reserved-identifier]]
[[js:module:ice]]

[[python:pkgdir:Ice]]

#include <Ice/BuiltinSequences.ice>
#include <Ice/Identity.ice>

[[java:package:com.zeroc]]
[cs:namespace:ZeroC]
module Ice
{
    // These definitions help with the encoding of requests.

    /// A request context. Each operation has a <code>Context</code> as its implicit final parameter.
    dictionary<string, string> Context;

    /// Determines the retry behavior an invocation in case of a (potentially) recoverable error. OperationMode is
    /// sent with each request to allow the server to verify the assumptions made by the caller.
    enum OperationMode : byte
    {
        /// Ordinary operations have <code>Normal</code> mode. These operations can modify object state; invoking such
        /// an operation twice in a row may have different semantics than invoking it once. The Ice run time guarantees
        /// that it will not violate at-most-once semantics for <code>Normal</code> operations.
        Normal,

        /// <p class="Deprecated"><code>Nonmutating</code> is deprecated; use <code>Idempotent</code> instead.
        Nonmutating,

        /// Operations that use the Slice <code>idempotent</code> keyword can modify object state, but invoking an
        /// operation twice in a row must result in the same object state as invoking it once. For example,
        /// <code>x = 1</code> is an idempotent statement, whereas <code>x += 1</code> is not. For idempotent
        /// operations, the Ice run-time does not guarantee at-most-once semantics.
        \Idempotent
    }

#ifdef __SLICE2CS__

    /// The priority of this request.
    // TODO: describe semantics.
    unchecked enum Priority : byte
    {
    }

    // A request header consists of two parts: a prologue which contains the frame type and frame size (and occasionally
    // more) and a body which contains the target's identity, operation name and more.

    [cs:readonly]
    struct Ice1RequestHeaderBody
    {
        Identity identity;
        StringSeq facetPath;
        string operation;
        OperationMode operationMode;
        Context context;
    }

    [cs:readonly]
    struct Ice2RequestHeaderBody
    {
        Identity identity;
        string? facet = "";      // null equivalent to empty string
        StringSeq? location;     // null equivalent to empty sequence
        string operation;
        bool \idempotent;
        Priority? priority;      // null equivalent to 0
    }
#endif
}
