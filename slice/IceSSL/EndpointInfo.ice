// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICESSL_API", "cpp:doxygen:include:IceSSL/IceSSL.h", "objc:header-dir:objc", "objc:dll-export:ICESSL_API",
  "python:pkgdir:IceSSL", "js:module:ice"]]

#include <Ice/Endpoint.ice>

/**
 *
 * IceSSL provides a secure transport for Ice.
 *
 **/
#ifndef __SLICE2JAVA_COMPAT__
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
}

}
