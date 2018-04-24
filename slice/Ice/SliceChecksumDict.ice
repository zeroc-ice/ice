// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICE_API", "cpp:doxygen:include:Ice/Ice.h", "objc:header-dir:objc", "objc:dll-export:ICE_API", "js:ice-build", "python:pkgdir:Ice"]]

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICE"]
module Ice
{

/**
 * A mapping from type IDs to Slice checksums. The dictionary
 * allows verification at run time that client and server
 * use matching Slice definitions.
 **/
dictionary<string, string> SliceChecksumDict;

}
