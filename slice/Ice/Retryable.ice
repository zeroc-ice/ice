//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[cpp:dll-export(ICE_API)]]
[[cpp:doxygen:include(Ice/Ice.h)]]
[[cpp:header-ext(h)]]
[[cpp:include(list)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(Ice)]]

[[java:package(com.zeroc)]]

[cs:namespace(ZeroC)]
module Ice
{
    /// The RetryAbility is carried by remote exceptions to indicate the ability for retrying
    enum Retryable : byte
    {
        /// do not retry
        No,
        /// retry same endpoint after delay ms
        AfterDelay,
        /// retry another replica known to the caller (if any)
        OtherReplica
    }
}
