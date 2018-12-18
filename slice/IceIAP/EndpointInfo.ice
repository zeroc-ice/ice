// **********************************************************************
//
// Copyright (c) present-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICEIAP_API", "cpp:doxygen:include:IceIAP/IceIAP.h", "objc:header-dir:objc", "js:module:ice"]]

#include <Ice/Endpoint.ice>

/**
 *
 * IceIAP provides a secure transport for Ice.
 *
 **/
["objc:prefix:ICEIAP"]
module IceIAP
{

/**
 *
 * Provides access to an IAP endpoint information.
 *
 **/
local class EndpointInfo extends Ice::EndpointInfo
{
    /**
     *
     * The accessory manufacturer or empty to not match against
     * a manufacturer.
     *
     **/
    string manufacturer;

    /**
     *
     * The accessory model number or empty to not match against
     * a model number.
     *
     **/
    string modelNumber;

    /**
     *
     * The accessory name or empty to not match against
     * the accessory name.
     *
     **/
    string name;

    /**
     *
     * The protocol supported by the accessory.
     *
     **/
    string protocol;
}

}
