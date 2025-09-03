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

/// The Ice RPC framework.
["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// Represents the identity of an Ice object. It is comparable to the path of a URI. Its string representation is
    /// `name` when the category is empty, and `category/name` when the category is not empty.
    ["cpp:custom-print"]
    struct Identity
    {
        /// The name of the Ice object. An empty name is not valid.
        string name;

        /// The category of the object.
        string category = "";
    }

    /// A sequence of identities.
    sequence<Identity> IdentitySeq;
}
