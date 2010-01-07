// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
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
 *
 * IceSSL provides a secure transport for Ice.
 *
 **/
module IceSSL
{

const short EndpointType = 2;

/**
 *
 * Provides access to an SSL endpoint information.
 *
 **/
local class EndpointInfo extends Ice::IPEndpointInfo
{
};

};

#endif
