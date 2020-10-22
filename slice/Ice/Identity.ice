//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    /// The identity of an Ice object.
    [cs:readonly]
    struct Identity
    {
        /// The name of the Ice object. An empty name is not a valid name.
        string name;

        /// The Ice object category.
        string category;
    }

    /// A sequence of identities.
    sequence<Identity> IdentitySeq;
}
