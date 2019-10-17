//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICESSL_API"]]
[["cpp:doxygen:include:IceSSL/IceSSL.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:dll-export:ICESSL_API"]]
[["objc:header-dir:objc"]]

[["python:pkgdir:IceSSL"]]

[["cpp:include:IceSSL/Plugin.h"]]

#include <Ice/Connection.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICESSL", "swift:module:Ice:SSL"]
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
     "cs:type:System.Security.Cryptography.X509Certificates.X509Certificate2[]",
     "swift:type:[SecCertificate]"]
    Ice::StringSeq certs;

    /** The certificate chain verification status. */
    bool verified;
}

}
