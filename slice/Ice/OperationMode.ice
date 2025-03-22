// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:Ice/StreamHelpers.h"]]

[["java:package:com.zeroc"]]
[["js:module:@zeroc/ice"]]
[["python:pkgdir:Ice"]]

module Ice
{
    // Note: The order of definitions here *must* match the order of definitions for ::Slice::Operation::Mode in
    // include/Slice/Parser.h!

    /// Determines the retry behavior an invocation in case of a (potentially) recoverable error.
    enum OperationMode
    {
        /// Ordinary operations have <code>Normal</code> mode. These operations modify object state; invoking such an
        /// operation twice in a row has different semantics than invoking it once. The Ice run time guarantees that it
        /// will not violate at-most-once semantics for <code>Normal</code> operations.
        ["swift:identifier:normal"]
        Normal,

        /// Operations that are <code>nonmutating</code> must not modify object state.
        /// The Ice run-time no longer makes a distinction between nonmutating operations and idempotent operations.
        /// Use the <code>idempotent</code> keyword instead.
        ["deprecated:Use Idempotent instead."]
        ["swift:identifier:nonmutating"]
        Nonmutating,

        /// Operations that use the Slice <code>idempotent</code> keyword can modify object state, but invoking an
        /// operation twice in a row must result in the same object state as invoking it once. For example,
        /// <code>x = 1</code> is an idempotent statement, whereas <code>x += 1</code> is not. In addition, the Ice
        /// run time will attempt to transparently recover from certain run-time errors by re-issuing a failed request
        /// and propagate the failure to the application only if the second attempt fails.
        ["swift:identifier:idempotent"]
        Idempotent
    }
}
