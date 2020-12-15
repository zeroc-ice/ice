/// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

[[cpp:dll-export(ICE_API)]]
[[cpp:doxygen:include(Ice/Ice.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(Ice)]]

[[java:package(com.zeroc)]]
[cs:namespace(ZeroC)]
module Ice
{
    /// A request context. Each operation has a <code>Context</code> as its implicit final parameter.
    [cs:generic:SortedDictionary] dictionary<string, string> Context;
}
