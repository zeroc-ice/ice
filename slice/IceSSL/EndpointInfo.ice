// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h"]]

#include <Ice/Endpoint.ice>

/**
 *
 * IceSSL provides a secure transport for Ice.
 *
 **/
module IceSSL
{

/**
 *
 * Uniquely identifies SSL endpoints.
 *
 **/
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

