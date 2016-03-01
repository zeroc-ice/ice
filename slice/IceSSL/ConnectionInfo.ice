// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]

#include <Ice/Connection.ice>

["objc:prefix:ICESSL"]
module IceSSL
{

/**
 *
 * Provides access to the connection details of an SSL connection
 *
 **/
local class ConnectionInfo extends Ice::IPConnectionInfo
{
    /** The negotiated cipher suite. */
    string cipher;

    /** The certificate chain. */
    Ice::StringSeq certs;

    /** The certificate chain verification status. */
    bool verified;
};

/**
 *
 * Provides access to the connection details of a secure WebSocket connection
 *
 **/
local class WSSConnectionInfo extends ConnectionInfo
{
    /** The headers from the HTTP upgrade request. */
    Ice::HeaderDict headers;
};

};

