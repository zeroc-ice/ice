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

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * A version structure for the protocol version.
 *
 **/
struct ProtocolVersion
{
    byte major;
    byte minor;
}

/**
 *
 * A version structure for the encoding version.
 *
 **/
struct EncodingVersion
{
    byte major;
    byte minor;
}

}
