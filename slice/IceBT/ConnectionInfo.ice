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

#include <Ice/Connection.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICEBT"]
module IceBT
{

/**
 *
 * Provides access to the details of a Bluetooth connection.
 *
 **/
local class ConnectionInfo extends Ice::ConnectionInfo
{
    /** The local Bluetooth address. */
    string localAddress = "";

    /** The local RFCOMM channel. */
    int localChannel = -1;

    /** The remote Bluetooth address. */
    string remoteAddress = "";

    /** The remote RFCOMM channel. */
    int remoteChannel = -1;

    /** The UUID of the service being offered (in a server) or targeted (in a client). */
    string uuid = "";

    /** The connection buffer receive size. **/
    int rcvSize = 0;

    /** The connection buffer send size. **/
    int sndSize = 0;
}

}
