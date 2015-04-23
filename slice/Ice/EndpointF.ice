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

["objc:prefix:ICE"]
module Ice
{

local class EndpointInfo;
local class IPEndpointInfo;
local class TCPEndpointInfo;
local class UDPEndpointInfo;
local class WSEndpointInfo;
local interface Endpoint;

/**
 *
 * A sequence of endpoints.
 *
 **/
local sequence<Endpoint> EndpointSeq;

};

