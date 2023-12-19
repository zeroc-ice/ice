//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICEBT_API"]]
[["cpp:doxygen:include:IceBT/IceBT.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]
[["js:cjs-module"]]

[["objc:header-dir:objc"]]

[["python:pkgdir:IceBT"]]

[["java:package:com.zeroc"]]

/// IceBT provides a Bluetooth transport for Ice.
["objc:prefix:ICEBT"]
module IceBT
{

/// Indicates a failure in the Bluetooth plug-in.
["cpp:ice_print"]
local exception BluetoothException
{
    /// Provides more information about the failure.
    string reason;
}

}
