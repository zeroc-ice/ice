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
#include <Ice/JavaCompat.ice>

/**
 *
 * IceSSL provides a secure transport for Ice.
 *
 **/
#ifndef JAVA_COMPAT
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICESSL"]
module IceSSL
{

/**
 *
 * Provides access to an SSL endpoint information.
 *
 **/
local class EndpointInfo extends Ice::EndpointInfo
{
};

};
