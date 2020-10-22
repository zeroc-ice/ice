// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

[[cpp:dll-export(ICE_API)]]
[[cpp:doxygen:include(Ice/Ice.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(Ice)]]

[cs:namespace(ZeroC)]
[java:package(com.zeroc)]
module Ice
{
    /// Represents a version of the Ice protocol.
    unchecked enum Protocol : byte
    {
        /// The ice1 protocol supported by all Ice versions since Ice 1.0.
        Ice1 = 1,
        /// The ice2 protocol introduced in Ice 4.0.
        Ice2 = 2
    }
}
