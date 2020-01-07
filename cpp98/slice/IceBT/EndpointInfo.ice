//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICEBT_API"]]
[["cpp:doxygen:include:IceBT/IceBT.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:header-dir:objc"]]

[["python:pkgdir:IceBT"]]

#include <Ice/Endpoint.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

/**
 *
 * IceBT provides a Bluetooth transport for Ice.
 *
 **/
["objc:prefix:ICEBT", "swift:module:Ice:SSL"]
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
