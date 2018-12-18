// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICEBT_API", "cpp:doxygen:include:IceBT/IceBT.h", "objc:header-dir:objc", "python:pkgdir:IceBT",
  "js:module:ice"]]

#include <Ice/Endpoint.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

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
}

}
