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
 * Indicates a failure in the Bluetooth plug-in.
 *
 **/
["cpp:ice_print"]
local exception BluetoothException
{
    /** Provides more information about the failure. */
    string reason;
}

}
