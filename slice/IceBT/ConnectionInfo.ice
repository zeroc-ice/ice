// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICEBT_API", "objc:header-dir:objc", "python:pkgdir:IceBT"]]

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
