// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:no-stream"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:Ice/Comparable.h"]]
[["cpp:include:cstdint"]]

[["js:module:ice"]]

[["python:pkgdir:Ice"]]

[["java:package:com.zeroc"]]

module Ice
{
    /// A version structure for the protocol version.
    struct ProtocolVersion
    {
        byte major;
        byte minor;
    }

    /// A version structure for the encoding version.
    struct EncodingVersion
    {
        byte major;
        byte minor;
    }
}
