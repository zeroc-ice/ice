// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICE_API", "objc:header-dir:objc", "objc:dll-export:ICE_API", "js:ice-build"]]

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
     * <tt>Random</tt> causes the endpoints to be arranged in a random order.
     */
    Random,
    /**
     * <tt>Ordered</tt> forces the Ice run time to use the endpoints in the
     * order they appeared in the proxy.
     */
    Ordered
};

};
