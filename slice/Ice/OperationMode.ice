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
    /// Specifies if an operation is idempotent, which affects the retry behavior of the Ice client runtime.
    enum OperationMode
    {
        /// A non-idempotent operation (the default). Invoking such an operation twice in a row has different semantics
        /// than invoking it once. The Ice runtime guarantees that it will not violate at-most-once semantics for
        /// operations with this mode.
        ["swift:identifier:normal"]
        Normal,

        /// Equivalent to {@link #Idempotent}, but deprecated.
        ["deprecated:Use Idempotent instead."]
        ["swift:identifier:nonmutating"]
        Nonmutating,

        /// An idempotent operation. Invoking such an operation twice in a row must result in the same object state
        /// as invoking it once. For example, `x = 1` is an idempotent statement, whereas `x += 1` is not.
        /// @remark When an operation is idempotent, the Ice runtime will attempt to transparently recover from certain
        /// runtime errors by re-issuing a failed request transparently.
        ["swift:identifier:idempotent"]
        Idempotent
    }
}
