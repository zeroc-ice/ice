// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_ENDPOINT_ICE
#define ICE_SSL_ENDPOINT_ICE

[["cpp:header-ext:h"]]

#include <Ice/Endpoint.ice>

module IceSSL
{

/**
 * SSL Endpoint
 **/
local interface SslEndpoint extends Ice::Endpoint
{
    /**
     * 
     * Get the host or address configured with
     * the endpoint.
     * 
     * @return The host or address.
     *
     **/
    ["cpp:const"] string host();

    /**
     * 
     * Get the TCP port number.
     *
     * @return The port number.
     * 
     **/
    ["cpp:const"] int port();
};

};

#endif
