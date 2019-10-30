//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICESSL_API"]]
[["cpp:doxygen:include:IceSSL/IceSSL.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["python:pkgdir:IceSSL"]]

[["cpp:include:IceSSL/Plugin.h"]]

#include <Ice/Connection.ice>

[["java:package:com.zeroc"]]

["swift:module:Ice:SSL"]
module IceSSL
{

/**
 *
 * Provides access to the connection details of an SSL connection
 *
 **/
local class ConnectionInfo extends Ice::ConnectionInfo
{
    /** The negotiated cipher suite. */
    string cipher;

    /** The certificate chain. */
    ["cpp:type:std::vector<CertificatePtr>",
     "java:type:java.security.cert.Certificate[]",
     "swift:type:[SecCertificate]"]
    Ice::StringSeq certs;

    /** The certificate chain verification status. */
    bool verified;
}

}
