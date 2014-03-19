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
 * IceWS provides a WebSocket transport for Ice.
 *
 **/
module IceWS
{

/**
 *
 * Uniquely identifies TCP-based WebSocket endpoints.
 *
 **/
const short WSEndpointType = 4;

/**
 *
 * Uniquely identifies SSL-based WebSocket endpoints.
 *
 **/
const short WSSEndpointType = 5;

/**
 *
 * Provides access to a WebSocket endpoint information.
 *
 **/
local class EndpointInfo extends Ice::IPEndpointInfo
{
    /**
     *
     * The URI configured with the endpoint.
     *
     **/
    string resource;
};

};

