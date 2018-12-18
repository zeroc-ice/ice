// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "cpp:dll-export:GLACIER2_API", "cpp:doxygen:include:Glacier2/Glacier2.h", "objc:header-dir:objc", "objc:dll-export:GLACIER2_API",
  "python:pkgdir:Glacier2", "js:module:ice"]]
[["cpp:include:Glacier2/Config.h"]]

#include <Ice/BuiltinSequences.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:GLACIER2"]
module Glacier2
{

/**
 *
 * Information taken from an SSL connection used for permissions
 * verification.
 *
 * @see PermissionsVerifier
 *
 */
struct SSLInfo
{
    /** The remote host. */
    string remoteHost;

    /** The remote port. */
    int remotePort;

    /** The router's host. */
    string localHost;

    /** The router's port. */
    int localPort;

    /** The negotiated cipher suite. */
    string cipher;

    /** The certificate chain. */
    Ice::StringSeq certs;
}

}
