// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:Ice/StreamHelpers.h"]]
[["cpp:include:Ice/TupleCompare.h"]]
[["cpp:include:string"]]
[["cpp:include:vector"]]

[["js:module:@zeroc/ice"]]

[["python:pkgdir:Ice"]]

[["java:package:com.zeroc"]]

/// Full-featured RPC framework.
module Ice
{
    /// The identity of an Ice object. In a proxy, an empty {@link Identity#name} denotes a null proxy. An identity with
    /// an empty {@link Identity#name} and a non-empty {@link Identity#category} is illegal. You cannot add a servant
    /// with an empty name to the Active Servant Map.
    /// @see ServantLocator
    /// @see ObjectAdapter#addServantLocator
    ["cpp:custom-print"]
    struct Identity
    {
        /// The name of the Ice object.
        string name;

        /// The Ice object category.
        string category = "";
    }

    /// A sequence of identities.
    sequence<Identity> IdentitySeq;
}
