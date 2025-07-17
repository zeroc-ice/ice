// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:cstdint"]]

[["js:module:@zeroc/ice"]]

// The endpoint types are called transport codes in IceRPC. They are used to marshal endpoints (as part of proxies) with
// the Slice 1.x encoding.

["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// Identifies endpoints marshaled as URI strings.
    const short URIEndpointType = 0;

    /// Identifies TCP endpoints.
    const short TCPEndpointType = 1;

    /// Identifies SSL endpoints.
    const short SSLEndpointType = 2;

    /// Identifies UDP endpoints.
    const short UDPEndpointType = 3;

    /// Identifies TCP-based WebSocket endpoints.
    const short WSEndpointType = 4;

    /// Identifies SSL-based WebSocket endpoints.
    const short WSSEndpointType = 5;

    /// Identifies Bluetooth endpoints.
    const short BTEndpointType = 6;

    /// Identifies SSL Bluetooth endpoints.
    const short BTSEndpointType = 7;

    /// Identifies iAP-based endpoints.
    const short iAPEndpointType = 8;

    /// Identifies SSL iAP-based endpoints.
    const short iAPSEndpointType = 9;
}
