// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICE_API", "cpp:doxygen:include:Ice/Ice.h", "objc:header-dir:objc", "objc:dll-export:ICE_API", "python:pkgdir:Ice", "js:module:ice"]]

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
