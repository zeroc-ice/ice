// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_ENDPOINT_INFO_ICE
#define ICE_SSL_ENDPOINT_INFO_ICE

[["cpp:header-ext:h"]]

#include <Ice/Endpoint.ice>

/**
 * IceSSL provides a secure transport for Ice.
 **/
module IceSSL
{

/**
 *
 * Provides access to the address details of an SSL endpoint.
 *
 **/
local class SSLEndpointInfo extends Ice::EndpointInfo
{
    /**
     * 
     * The host or address configured with the endpoint.
     *
     **/
    string host;

    /**
     * 
     * The TCP port number.
     * 
     **/
    int port;
};

};

#endif
