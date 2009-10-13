// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTION_INFO_ICE
#define ICE_SSL_CONNECTION_INFO_ICE

[["cpp:header-ext:h"]]

#include <Ice/Connection.ice>

module IceSSL
{

/**
 *
 * Provides access to the connection details of an SSL connection
 *
 **/
local class SSLConnectionInfo extends Ice::ConnectionInfo
{
    /** The local address. */
    string localAddress;

    /** The local port. */
    int localPort;

    /** The remote address. */
    string remoteAddress;

    /** The remote port. */
    int remotePort;    

    /** The negotiated cipher suite. */
    string cipher;

    /** The certificate chain. */
    Ice::StringSeq certs;
};

};

#endif
