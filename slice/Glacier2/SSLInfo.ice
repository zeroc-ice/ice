// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]
[["cpp:include:Glacier2/Config.h"]]

#include <Ice/BuiltinSequences.ice>

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
};

};

