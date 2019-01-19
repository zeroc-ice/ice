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
