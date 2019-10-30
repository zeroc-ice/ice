//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICEBT_API"]]
[["cpp:doxygen:include:IceBT/IceBT.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["python:pkgdir:IceBT"]]

#include <Ice/Endpoint.ice>

[["java:package:com.zeroc"]]

/**
 *
 * IceBT provides a Bluetooth transport for Ice.
 *
 **/
["swift:module:Ice:SSL"]
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
