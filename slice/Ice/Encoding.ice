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
    /// The Ice encoding defines how Slice constructs are marshaled to and later unmarshaled from sequences of bytes.
    /// An Encoding struct holds a version of the Ice encoding.
    [cs:readonly]
    struct Encoding
    {
        /// The major version number of this version of the Ice encoding.
        byte major;

        /// The minor version number of this version of the Ice encoding.
        byte minor;
    }
}
