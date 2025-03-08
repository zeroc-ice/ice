// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:cstdint"]]

[["js:module:@zeroc/ice"]]

[["python:pkgdir:Ice"]]

[["java:package:com.zeroc"]]

module Ice
{
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
