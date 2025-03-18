// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:no-stream"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:Ice/TupleCompare.h"]]
[["cpp:include:cstdint"]]
[["cpp:include:ostream"]]

[["js:module:@zeroc/ice"]]

[["python:pkgdir:Ice"]]

[["java:package:com.zeroc"]]

module Ice
{
    /// A version structure for the protocol version.
    ["cpp:custom-print"]
    struct ProtocolVersion
    {
        /// The major version of the Ice protocol.
        byte major;

        /// The minor version of the Ice protocol.
        byte minor;
    }

    /// A version structure for the encoding version.
    ["cpp:custom-print"]
    struct EncodingVersion
    {
        /// The major version of the Ice encoding.
        byte major;

        /// The minor version of the Ice encoding.
        byte minor;
    }
}
