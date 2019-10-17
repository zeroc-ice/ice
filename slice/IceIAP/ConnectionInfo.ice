//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICEIAP_API"]]
[["cpp:doxygen:include:IceIAP/IceIAP.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:header-dir:objc"]]

#include <Ice/Connection.ice>

["objc:prefix:ICEIAP", "swift:module:Ice:IAP"]
module IceIAP
{

/**
 *
 * Provides access to the connection details of an IAP connection
 *
 **/
local class ConnectionInfo extends Ice::ConnectionInfo
{
    /**
     *
     * The accessory name.
     *
     **/
    string name;

    /**
     *
     * The accessory manufacturer.
     *
     **/
    string manufacturer;

    /**
     *
     * The accessory model number.
     *
     **/
    string modelNumber;

    /**
     *
     * The accessory firmare revision.
     *
     **/
    string firmwareRevision;

    /**
     *
     * The accessory hardware revision.
     *
     **/
    string hardwareRevision;

    /**
     *
     * The protocol used by the accessory.
     *
     **/
    string protocol;
}

}
