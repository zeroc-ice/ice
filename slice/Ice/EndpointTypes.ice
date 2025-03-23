// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:cstdint"]]

[["java:package:com.zeroc"]]
[["js:module:@zeroc/ice"]]
[["python:pkgdir:Ice"]]

// The endpoint types are called transport codes in IceRPC. They are used to encode endpoints (as part of proxies) with
// the Slice 1.x encoding.

module Ice
{
    /// Uniquely identifies endpoints encoded as URI strings. Currently, Ice cannot parse such endpoints.
    const short UriEndpointType = 0;

    /// Uniquely identifies TCP endpoints.
    const short TCPEndpointType = 1;

    /// Uniquely identifies SSL endpoints.
    const short SSLEndpointType = 2;

    /// Uniquely identifies UDP endpoints.
    const short UDPEndpointType = 3;

    /// Uniquely identifies TCP-based WebSocket endpoints.
    const short WSEndpointType = 4;

    /// Uniquely identifies SSL-based WebSocket endpoints.
    const short WSSEndpointType = 5;

    /// Uniquely identifies Bluetooth endpoints.
    const short BTEndpointType = 6;

    /// Uniquely identifies SSL Bluetooth endpoints.
    const short BTSEndpointType = 7;

    /// Uniquely identifies iAP-based endpoints.
    const short iAPEndpointType = 8;

    /// Uniquely identifies SSL iAP-based endpoints.
    const short iAPSEndpointType = 9;
}
