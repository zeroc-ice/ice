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

#include <Ice/Endpoint.ice>

/**
 *
 * IceBT provides a Bluetooth transport for Ice.
 *
 **/
["objc:prefix:ICEBT"]
module IceBT
{

/**
 *
 * Uniquely identifies Bluetooth endpoints.
 *
 **/
#ifndef __SLICE2OBJC__
const short EndpointType = 6;
#endif

/**
 *
 * Provides access to Bluetooth endpoint information.
 *
 **/
local class EndpointInfo extends Ice::EndpointInfo
{
    /**
     *
     * The address configured with the endpoint.
     *
     **/
    string addr;

    /**
     *
     * The UUID configured with the endpoint.
     *
     **/
    string uuid;
};

};
