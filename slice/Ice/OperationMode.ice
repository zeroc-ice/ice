// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:Ice/StreamHelpers.h"]]

[["js:module:@zeroc/ice"]]

["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// Specifies if an operation is idempotent, which affects the retry behavior of the Ice client runtime.
    enum OperationMode
    {
        /// A non-idempotent operation (the default). The Ice client runtime guarantees that it will not violate
        /// at-most-once semantics for operations with this mode.
        ["swift:identifier:normal"]
        Normal,

        /// Equivalent to {@link #Idempotent}, but deprecated.
        ["deprecated:Use Idempotent instead."]
        ["swift:identifier:nonmutating"]
        Nonmutating,

        /// An idempotent operation. The Ice client runtime does not guarantee at-most-once semantics for such an
        /// operation.
        /// @remark When an operation is idempotent, the Ice runtime will attempt to transparently recover from certain
        /// runtime errors by re-issuing a failed request transparently.
        ["swift:identifier:idempotent"]
        Idempotent
    }
}
