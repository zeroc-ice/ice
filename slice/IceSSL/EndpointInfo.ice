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

#include <Ice/Endpoint.ice>

/**
 *
 * IceSSL provides a secure transport for Ice.
 *
 **/
["objc:prefix:ICESSL"]
module IceSSL
{

/**
 *
 * Uniquely identifies SSL endpoints.
 *
 **/
#ifndef __SLICE2OBJC__ // In Objective-C, Ice::SSLEndpointType is already mapped to ICESSLEndpointType
const short EndpointType = Ice::SSLEndpointType;
#endif

/**
 *
 * Provides access to an SSL endpoint information.
 *
 **/
local class EndpointInfo extends Ice::IPEndpointInfo
{
};

/**
 *
 * Provides access to a secure WebSocket endpoint information.
 *
 **/
local class WSSEndpointInfo extends EndpointInfo
{
    /**
     *
     * The URI configured with the endpoint.
     *
     **/
    string resource;
};

};

