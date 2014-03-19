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

module Ice
{

local class EndpointInfo;
local class TCPEndpointInfo;
local class UDPEndpointInfo;
local interface Endpoint;

/**
 *
 * A sequence of endpoints.
 *
 **/
local sequence<Endpoint> EndpointSeq;

};

