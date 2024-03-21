//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:Ice/Comparable.h"]]
[["cpp:include:Ice/StreamHelpers.h"]]
[["cpp:include:cstdint"]]

[["ice-prefix"]]

[["js:module:ice"]]
[["js:cjs-module"]]

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

}
