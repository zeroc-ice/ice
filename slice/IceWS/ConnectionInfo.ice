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

#include <Ice/Connection.ice>

module IceWS
{

/**
 *
 * Provides access to the connection details of a WebSocket connection
 *
 **/
local class ConnectionInfo extends Ice::IPConnectionInfo
{
};

};

