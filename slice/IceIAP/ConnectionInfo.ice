// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICEIAP_API", "cpp:doxygen:include:IceIAP/IceIAP.h", "objc:header-dir:objc", "js:ice-build"]]

#include <Ice/Connection.ice>

["objc:prefix:ICEIAP"]
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
