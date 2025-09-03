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

["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// Represents a version of the Ice protocol. The only version implemented and supported by Ice is version 1.0.
    ["cpp:custom-print"]
    struct ProtocolVersion
    {
        /// The major version of the Ice protocol.
        byte major;

        /// The minor version of the Ice protocol.
        byte minor;
    }

    /// Represents a version of the Slice encoding. Ice supports version 1.0 and 1.1 of this encoding.
    /// @remark The Slice encoding is also known as the Ice encoding.
    ["cpp:custom-print"]
    struct EncodingVersion
    {
        /// The major version of the Slice encoding.
        byte major;

        /// The minor version of the Slice encoding.
        byte minor;
    }
}
