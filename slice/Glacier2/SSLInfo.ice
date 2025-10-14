// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:GLACIER2_API"]]
[["cpp:doxygen:include:Glacier2/Glacier2.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:Glacier2/Config.h"]]

[["js:module:@zeroc/ice"]]

#include "Ice/BuiltinSequences.ice"

["java:identifier:com.zeroc.Glacier2"]
module Glacier2
{
    interface SSLPermissionsVerifier; // So that doc-comments can link to `SSLPermissionsVerifier`.

    /// Represents information gathered from an incoming SSL connection and used for authentication and authorization.
    /// @see SSLPermissionsVerifier
    struct SSLInfo
    {
        /// The remote host.
        string remoteHost;

        /// The remote port.
        int remotePort;

        /// The router's host.
        string localHost;

        /// The router's port.
        int localPort;

        /// The negotiated cipher suite.
        string cipher;

        /// The certificate chain.
        Ice::StringSeq certs;
    }
}
