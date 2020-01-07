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

#include <Ice/Endpoint.ice>

/**
 *
 * IceIAP provides a secure transport for Ice.
 *
 **/
["objc:prefix:ICEIAP", "swift:module:Ice:IAP"]
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
