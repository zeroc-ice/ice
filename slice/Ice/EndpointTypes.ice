// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
 * Determines the order in which the Ice run time uses the endpoints
 * in a proxy when establishing a connection.
 *
 **/
local enum EndpointSelectionType
{
    /**
     * <code>Random</code> causes the endpoints to be arranged in a random order.
     */
    Random,
    /**
     * <code>Ordered</code> forces the Ice run time to use the endpoints in the
     * order they appeared in the proxy.
     */
    Ordered
}

}
