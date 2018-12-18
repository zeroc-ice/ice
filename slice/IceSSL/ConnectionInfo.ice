// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICESSL_API", "cpp:doxygen:include:IceSSL/IceSSL.h", "objc:header-dir:objc", "objc:dll-export:ICESSL_API",
  "python:pkgdir:IceSSL", "js:module:ice"]]

[["cpp:include:IceSSL/Plugin.h"]]

#include <Ice/Connection.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICESSL"]
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
     "cs:type:System.Security.Cryptography.X509Certificates.X509Certificate2[]"]
    Ice::StringSeq certs;

    /** The certificate chain verification status. */
    bool verified;
}

}
